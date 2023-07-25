#include <xx_lua_data.h>
using LuaStateWithInt = xx::Lua::StateWithExtra<int>;
#define RefWeakTableRefId(L) LuaStateWithInt::Extra(L)
#include <xx_lua_shared.h>
namespace xl = xx::Lua;


#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

EM_JS(void, PrintValHandle, (emscripten::EM_VAL h), {
    console.log(Emval.toValue(h));
});

void PrintVal(emscripten::val const& v) {
    PrintValHandle(v.as_handle());
}


int main() {
    emscripten_run_script(R"(

function Foo() {
    console.log('js: Foo()');
    this.name = 'foo';
}
Foo.prototype.GetName = function() {
    console.log('js: GetName()');
    return this.name;
};
Foo.prototype.SetName = function(name) {
    console.log('js: SetName(name) ' + name);
    this.name = name;
};

Bar = {};
Bar.Add = function(a, b) {
    console.log('js: Bar.Add(a, b)');
    console.log('js: a = ' + a);
    console.log('js: b = ' + b);
    return a + b;
};

)");

    LuaStateWithInt L;
    xl::MakeUserdataWeakTable(L);
    xl::RegisterBaseEnv(L);

    xx_assert(lua_gettop(L) == 0);
    lua_newtable(L);                                            // ..., t
    xx_assert(lua_gettop(L) == 1);
    xl::SetField(L, "_tablename_", "Bar");                            // store tablename
    lua_newtable(L);                                            // ..., t, mt
    xx_assert(lua_gettop(L) == 2);
    xl::SetFieldCClosure(L, "__index", [](auto L)->int{
        lua_getfield(L, 1, "_tablename_");                                          // t, k, tablename
        xx_assert(lua_type(L, -1) == LUA_TSTRING);
        // todo: find k in tablename ??
        lua_pushcclosure(L, [](lua_State*L)->int{
            auto member = xl::To<std::string_view>(L, lua_upvalueindex(1));
            auto tablename = xl::To<std::string_view>(L, lua_upvalueindex(2));
            std::cout << "lua: call " << tablename << "." << member << std::endl;
            // todo: forward send arguments?
            auto a1 = xl::To<int>(L, 1);
            auto a2 = xl::To<int>(L, 2);
            auto r = emscripten::val::global(tablename.data()).call<emscripten::val>(member.data(), a1, a2);
            // todo: switch case r type? convert to lua value?
            return xl::Push(L, r.as<int>());
        }, 2);                                                                      // t, c
        return 1;
    });
    xx_assert(lua_gettop(L) == 2);
    lua_setmetatable(L, -2);                                    // ..., t
    xx_assert(lua_gettop(L) == 1);
    lua_setglobal(L, "Bar");                                    // ...
    xx_assert(lua_gettop(L) == 0);

    xl::DoString(L, R"(

print(Bar)
print(Bar.Add)
print(Bar.Add(1, 2))

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
