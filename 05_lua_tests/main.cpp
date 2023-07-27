#include <xx_lua_data.h>
namespace xl = xx::Lua;
//using LuaStateWithInt = xx::Lua::StateWithExtra<int>;
//#define RefWeakTableRefId(L) LuaStateWithInt::Extra(L)
//#include <xx_lua_shared.h>
//LuaStateWithInt L;
//xl::MakeUserdataWeakTable(L);
//xl::RegisterBaseEnv(L);

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
using V = emscripten::val;

EM_JS(void, PrintValHandle, (emscripten::EM_VAL h), {
    console.log(Emval.toValue(h));
});

void PrintVal(V const& v) {
    PrintValHandle(v.as_handle());
}

EM_JS(bool, IsFunction_, (emscripten::EM_VAL h), {
    let tn = typeof(Emval.toValue(h));
    console.log('js: IsFunction_ tn == ' + tn);
    return tn == 'function';
});

bool IsFunction(V const& v) {
    return IsFunction_(v.as_handle());
}


std::vector<V> gVals;
std::vector<V*> gValptrs;
std::unordered_map<std::string, double> gMap;
// todo: cache metatable to registry

void SetValMeta(lua_State* L);
int HandleVal(lua_State* L, V& m);

int PushValFunction(lua_State* L, V& m) {
    new(lua_newuserdata(L, sizeof(V))) V(std::move(m));
    lua_newtable(L);
    xx::Lua::SetFieldCClosure(L, "__gc", [](lua_State* L)->int {
        ((V*)lua_touserdata(L, 1))->~V();
        return 0;
    });
    lua_setmetatable(L, -2);

    lua_pushcclosure(L, [](lua_State*L)->int{
        auto& f = *(V*)lua_touserdata(L, lua_upvalueindex(1));

        auto n = lua_gettop(L);

        gVals.resize(n);
        gValptrs.resize(n);

        for(int i = 0; i < n; ++i) {
            auto idx = i + 1;
            auto t = lua_type(L, idx);
//                std::cout << "before switch t = " << t << std::endl;
            switch(t) {
                case LUA_TNIL:
                    gVals[i] = V::null();
                    gValptrs[i] = &gVals[i];
                    break;
                case LUA_TBOOLEAN:
                    gVals[i] = V(xl::To<bool>(L, idx));
                    gValptrs[i] = &gVals[i];
                    break;
                case LUA_TLIGHTUSERDATA:
                    xx_assert(false);
                case LUA_TNUMBER:
                    gVals[i] = V(xl::To<double>(L, idx));
                    gValptrs[i] = &gVals[i];
                    break;
                case LUA_TSTRING:
                    gVals[i] = V(xl::To<char const*>(L, idx));
                    gValptrs[i] = &gVals[i];
                    break;
                case LUA_TTABLE: {
                    xl::To(L, idx, gMap);
                    xx_assert(!gMap.empty());
                    auto &o = gVals[i];
                    gValptrs[i] = &o;
                    o = V::object();
                    for (auto &kv: gMap) {
                        o.set(kv.first, kv.second);
                    }
                    gMap.clear();
                    break;
                }
                case LUA_TFUNCTION:
                    // todo: emscripten_run_script create global function with unique name, then val get it ? how to gc ?
                    xx_assert(false);
                case LUA_TUSERDATA:
                    gValptrs[i] = (V*)lua_touserdata(L, idx);
                    break;
                case LUA_TTHREAD:
                    xx_assert(false);
                default:
                    xx_assert(false);
            }
        }

        V r;
        switch(n) {
            case 0:
                r = f();
                break;
            case 1:
                r = f(*gValptrs[0]);
                break;
            case 2:
                r = f(*gValptrs[0], *gValptrs[1]);
                break;
            case 3:
                r = f(*gValptrs[0], *gValptrs[1], *gValptrs[2]);
                break;
            case 4:
                r = f(*gValptrs[0], *gValptrs[1], *gValptrs[2], *gValptrs[3]);
                break;
            case 5:
                r = f(*gValptrs[0], *gValptrs[1], *gValptrs[2], *gValptrs[3], *gValptrs[4]);
                break;
                // known issue: add more ?
            default:
                xx_assert(false);
        }
        gVals.clear();

        return HandleVal(L, r);
    }, 1);

    return 1;
}

int HandleVal(lua_State* L, V& m) {
    if (m.isNull() || m.isUndefined()) return 0;
    else if (m.isTrue()) return xl::Push(L, true);
    else if (m.isFalse()) return xl::Push(L, false);
    else if (m.isNumber()) {
        auto v = m.template as<double>();
        if ((int64_t)v == v) return xl::Push(L, (int64_t)v);
        else return xl::Push(L, v);
    }
    else if (m.isString()) return xl::Push(L, m.template as<std::string>());
    else if (IsFunction(m)) {
        return PushValFunction(L, m);
    }
    else if (m.isArray()) {
        // todo?
        xx_assert(false);
    } else {
        new(lua_newuserdata(L, sizeof(V))) V(std::move(m));
        SetValMeta(L);
    }
    return 1;
}

void SetValMeta(lua_State* L) {
    auto top = lua_gettop(L);
    xx_assert(top >= 1);
    xx_assert(lua_type(L, top) == LUA_TUSERDATA);
    lua_newtable(L);                                            // ..., ud, mt

    xx::Lua::SetFieldCClosure(L, "__gc", [](lua_State* L)->int {
        ((V*)lua_touserdata(L, 1))->~V();
        return 0;
    });

    xl::SetFieldCClosure(L, "__index", [](auto L)->int{
        auto p = (V*)lua_touserdata(L, 1);
        auto memberName = xl::To<char const*>(L, 2);

        auto m = (*p)[memberName];
        if (m.isNull() || m.isUndefined()) return 0;

        return HandleVal(L, m);
    });

    lua_setmetatable(L, -2);                                    // ..., ud
    xx_assert(lua_gettop(L) == top);
}

int main() {
    emscripten_run_script(R"(

a = function() {};
b = new Object;
c = { x:1, y:2 };
d = [ 1, 2 ];
e = 123;
console.log( typeof(a) )
console.log( typeof(b) )
console.log( typeof(c) )
console.log( typeof(d) )

Bar = {};
Bar.Add = function(a, b) {
    return a + b;
};
Bar.CreateObject = function( name ) {
    return { "name":name };
};
Bar.Log = function( o ) {
    console.log( o );
};

p = { x:1, y:2 };

)");

    xl::State L;

    xl::SetGlobalCClosure(L, "FromJS", [](auto L){
        auto name = xl::To<char const*>(L, 1);
        auto v = V::global(name);
        return HandleVal(L, v);
    });

    xl::DoString(L, R"(
local c = FromJS( "c" )
print( c )

local e = FromJS( "e" )
print( e )

local b = FromJS( "Bar" )
print( b )
local f =  b.Add
print( f )
--[[
b.Add = function()
    print( "new func" )
end
]]
print( f(1, 2) )

local o = b.CreateObject( "asdf" )
print( o )
b.Log( o )

local p = FromJS( "p" )
print( p.x )

)");

//    xl::DoFile(L, "/res/test1.lua");
    return 0;
}


//int main() {
//    LuaStateWithInt L;
//    xl::SetGlobalCClosure(L, "IndexFunc", [](lua_State*L)->int{
//        auto k = xl::To<std::string_view>(L, 2);
//        std::cout << k << std::endl;
//        if (k == "abc"sv) {
//            lua_pushcclosure(L, [](lua_State*L)->int{
//                std::cout << "abc func has been called by args == " << xl::To<int>(L, 1) << " " << xl::To<std::string_view>(L, 2) << std::endl;
//                return 0;
//            }, 0);
//            return 1;
//        }
//        return 0;
//    });
//    xl::DoString(L, R"(
//    local t = {}
//    local m = {}
//    m.__index = IndexFunc
//    setmetatable(t, m)
//    t.abc(123, "asdf")
//)");
//    return 0;
//}



//#include <xx_dict.h>
//#include <xx_list.h>
//int main() {
//    auto secs = xx::NowEpochSeconds();
//
//    // lua msvc exe                         // 0.19
//    // js                                   // 0.17
//
//    // lua                                  // 0.31     0.43 ( -sALLOW_MEMORY_GROWTH=1 )
//
//    //std::unordered_map<int, int> ints;    // 0.267
//
//    //xx::Dict<int, int> ints;              // 0.16
//    //ints.Reserve(10000000);
//
//    //std::vector<int> ints;                // 0.015
//    //ints.resize(10000000);
//
//    xx::List<int> ints;                     // 0.013
//    ints.Resize(10000000);
//
//    int i = 0;
//    for (; i < 10000000; ++i) {
//        ints[i] = i;
//    }
//    std::cout << xx::NowEpochSeconds(secs) << std::endl;
//    std::cout << ints[0] << ints[i/2] << std::endl;
//    return 0;
//}
