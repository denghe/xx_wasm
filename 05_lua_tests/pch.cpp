#include "pch.h"
//using LuaStateWithInt = xx::Lua::StateWithExtra<int>;
//#define RefWeakTableRefId(L) LuaStateWithInt::Extra(L)
//#include <xx_lua_shared.h>
//LuaStateWithInt L;
//xl::MakeUserdataWeakTable(L);
//xl::RegisterBaseEnv(L);

EM_JS(void, PrintValHandle, (emscripten::EM_VAL h), {
    console.log(Emval.toValue(h));
})

void PrintVal(V const& v) {
    PrintValHandle(v.as_handle());
}


xl::State gL;
V gNilVal;
std::vector<V> gVals;
std::vector<V*> gValptrs;
std::unordered_map<std::string, double> gMap;


// lua stack -> gValptrs
void ToVals(lua_State* L, int beginIndex, int size) {
    gVals.clear();
    gVals.resize(size);
    gValptrs.resize(size);

    for (int i = 0; i < size; ++i) {
        auto idx = beginIndex + i;
        auto t = lua_type(L, idx);
        switch (t) {
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
            break;
        case LUA_TNUMBER:
            gVals[i] = V(xl::To<double>(L, idx));
            gValptrs[i] = &gVals[i];
            break;
        case LUA_TSTRING:
            gVals[i] = V(xl::To<std::string>(L, idx));
            gValptrs[i] = &gVals[i];
            break;
        case LUA_TTABLE: {
            xl::To(L, idx, gMap);
            xx_assert(!gMap.empty());
            auto& o = gVals[i];
            gValptrs[i] = &o;
            o = V::object();
            for (auto& kv : gMap) {
                o.set(kv.first, kv.second);
            }
            gMap.clear();
            break;
        }
        case LUA_TFUNCTION: {
            lua_pushvalue(L, idx);                          // ..., f, ..., f
            auto k = luaL_ref(L, LUA_REGISTRYINDEX);        // ..., f, ...
            auto s = std::string("CppCallback = (...args)=>{ return WasmCallback(" + std::to_string(k) + ", args); };");
            emscripten_run_script(s.c_str());   // todo: gc?
            gVals[i] = emscripten::val::global("CppCallback");
            gValptrs[i] = &gVals[i];
            break;
        }
        case LUA_TUSERDATA:
            gValptrs[i] = (V*)lua_touserdata(L, idx);
            break;
        case LUA_TTHREAD:
            xx_assert(false);
            break;
        default:
            printf("unsupported t == %d\n", (int)t);
            xx_assert(false);
        }
    }
}



// return true if tar is Object|Function
bool PushVal(lua_State* L, V& tar) {
    if (tar.isNull() || tar.isUndefined()) {
        lua_pushnil(L);
    } else if (tar.isTrue()) {
        xl::Push(L, true);
    } else if (tar.isFalse()) {
        xl::Push(L, false);
    } else if (tar.isNumber()) {
        auto v = tar.template as<double>();
        if ((int64_t)v == v) {
            xl::Push(L, (int64_t)v);
        } else {
            xl::Push(L, v);
        }
    } else if (tar.isString()) {
        xl::Push(L, tar.template as<std::string>());
    } else if (tar.isArray()) {
        xx_assert(false);   // todo?
    } else return true;
    return false;
}

void SetValMeta(lua_State* L);

template<bool direct>
int CallVal(lua_State* L) {
    auto v = (V*)lua_touserdata(L, 1);
    int numArgs = lua_gettop(L) - 1, startIndex = 2;
    // auto remove lua first args : self
    if (lua_type(L, startIndex) == LUA_TUSERDATA && lua_touserdata(L, startIndex) == (void*)v) {
        ++startIndex;
        --numArgs;
    }
    V r;
    if constexpr (direct) {                                                     // ud, args...
        ToVals(L, startIndex, numArgs);
        auto v = (V*)lua_touserdata(L, 1);                                      // ud is v
        switch(numArgs) {
            case 0:
                r = (*v)();
                break;
            case 1:
                r = (*v)(*gValptrs[0]);
                break;
            case 2:
                r = (*v)(*gValptrs[0], *gValptrs[1]);
                break;
            case 3:
                r = (*v)(*gValptrs[0], *gValptrs[1], *gValptrs[2]);
                break;
            case 4:
                r = (*v)(*gValptrs[0], *gValptrs[1], *gValptrs[2], *gValptrs[3]);
                break;
            case 5:
                r = (*v)(*gValptrs[0], *gValptrs[1], *gValptrs[2], *gValptrs[3], *gValptrs[4]);
                break;
            default:
                printf("call val need add more case\n");
                xx_assert(false);
        }
    } else {                                                                    // ud, args..., owner, memberName
        numArgs -= 2;
        ToVals(L, startIndex, numArgs);
        auto v = (V*)lua_touserdata(L, -2);                                     // ud's owner.member is v
        auto memberName = xl::To<char const*>(L, -1);
        switch (numArgs) {
        case 0:
            r = v->template call<V>(memberName);
            break;
        case 1:
            r = v->template call<V>(memberName, *gValptrs[0]);
            break;
        case 2:
            r = v->template call<V>(memberName, *gValptrs[0], *gValptrs[1]);
            break;
        case 3:
            r = v->template call<V>(memberName, *gValptrs[0], *gValptrs[1], *gValptrs[2]);
            break;
        case 4:
            r = v->template call<V>(memberName, *gValptrs[0], *gValptrs[1], *gValptrs[2], *gValptrs[3]);
            break;
        case 5:
            r = v->template call<V>(memberName, *gValptrs[0], *gValptrs[1], *gValptrs[2], *gValptrs[3], *gValptrs[4]);
            break;
        default:
            printf("call val need add more case\n");
            xx_assert(false);
        }
    }
    if (PushVal(L, r)) {
        new(lua_newuserdatauv(L, sizeof(V), 2)) V(std::move(r));
        SetValMeta(L);
    }
    return 1;
}

// todo: cache metatable to registry
void SetValMeta(lua_State* L) {
    auto top = lua_gettop(L);
    xx_assert(top >= 1);
    xx_assert(lua_type(L, top) == LUA_TUSERDATA);
    lua_newtable(L);                                            // ..., ud, mt

    xx::Lua::SetFieldCClosure(L, "__gc", [](lua_State* L)->int {        // ud
        ((V*)lua_touserdata(L, 1))->~V();
        return 0;
    });

    xl::SetFieldCClosure(L, "__index", [](auto L)->int{                     // ud, mn
        auto v = (V*)lua_touserdata(L, 1);
        auto memberName = xl::To<std::string>(L, 2);
        auto m = (*v)[memberName];
        if (m.isNull() || m.isUndefined()) {    // first letter : lower case / upper case compatible
            auto s = memberName;    // copy
            if (auto c = s[0]; c >= 'a' && c <= 'z') {
                s[0] = (char)std::toupper(c);
            } else if (c >= 'A' && c <= 'Z') {
                s[0] = (char)std::tolower(c);
            } else return 0;

            m = (*v)[s.c_str()];
            if (m.isNull() || m.isUndefined()) return 0;
            memberName = s;
            lua_pop(L, 1);                                                  // ud
            xl::Push(L, s);                                                 // ud, correct mn
        }
        if (PushVal(L, m)) {                                                // ud, mn, m?
            new(lua_newuserdatauv(L, sizeof(V), 2)) V(std::move(m));        // ud, mn, m
            lua_pushvalue(L, 1);                                            // ud, mn, m, ud
            lua_setiuservalue(L, 3, 1);                                     // ud, mn, m
            lua_pushvalue(L, 2);                                            // ud, mn, m, mn
            lua_setiuservalue(L, 3, 1);                                     // ud, mn, m
            SetValMeta(L);
        }
        return 1;
    });

    xl::SetFieldCClosure(L, "__newindex", [](auto L)->int{
        auto v = (V*)lua_touserdata(L, 1);
        auto memberName = xl::To<char const*>(L, 2);
        auto t = lua_type(L, 3);
        switch(t) {
            case LUA_TNIL:
                v->set(memberName, V::null());
                break;
            case LUA_TBOOLEAN:
                v->set(memberName, xl::To<bool>(L, 3));
                break;
            case LUA_TLIGHTUSERDATA:
                xx_assert(false);
            case LUA_TNUMBER:
                v->set(memberName, xl::To<double>(L, 3));
                break;
            case LUA_TSTRING:
                v->set(memberName, xl::To<char const*>(L, 3));
                break;
            case LUA_TTABLE: {
                xl::To(L, 3, gMap);
                xx_assert(!gMap.empty());
                auto o = V::object();
                for (auto &kv: gMap) {
                    o.set(kv.first, kv.second);
                }
                gMap.clear();
                v->set(memberName, o);
                break;
            }
            case LUA_TFUNCTION: {
                auto k = luaL_ref(L, LUA_REGISTRYINDEX);
                auto s = std::string("CppCallback = (...args)=>{ return WasmCallback(" + std::to_string(k) + ", args); };");
                emscripten_run_script(s.c_str());   // todo: gc?
                v->set(memberName, emscripten::val::global("CppCallback"));
                break;
            }
            case LUA_TUSERDATA:
                v->set(memberName, *(V*)lua_touserdata(L, 3));
                break;
            case LUA_TTHREAD:
                xx_assert(false);
            default:
                printf("unsupported t == %d\n", (int)t);
                xx_assert(false);
        }
        return 0;
    });

    xl::SetFieldCClosure(L, "__call", [](auto L)->int {
        if (LUA_TNONE == lua_getiuservalue(L, 1, 1)) {          // ud, args..., owner?
            lua_pop(L, 1);                                      // ud, args...
            return CallVal<true>(L);
        } else {
            lua_getiuservalue(L, 1, 2);                         // ud, args..., owner, memberName
            return CallVal<false>(L);
        }
    });

    lua_setmetatable(L, -2);                                    // ud
    xx_assert(lua_gettop(L) == top);
}


// js need global function: WasmCallback => wasm.Callback
emscripten::val const& Callback(int key, emscripten::val const& a) {
    auto& L = gL;
    auto top = lua_gettop(L);
    lua_rawgeti(L, LUA_REGISTRYINDEX, key);             // ..., func
    int n = a["length"].as<int>();
    for (int i = 0; i < n; ++i) {                       // ..., func, args...
        auto&& m = a[i];
        if (m.isNull() || m.isUndefined()) {
            lua_pushnil(L);
        } else if (m.isTrue()) {
            xl::Push(L, true);
        } else if (m.isFalse()) {
            xl::Push(L, false);
        } else if (m.isNumber()) {
            auto v = m.template as<double>();
            if ((int64_t)v == v) xl::Push(L, (int64_t)v);
            else xl::Push(L, v);
        } else if (m.isString()) {
            xl::Push(L, m.template as<std::string>());
        } else if (m.isArray()) {
            // todo?
            xx_assert(false);
        } else {
            new(lua_newuserdatauv(L, sizeof(V), 2)) V(std::move(m));
            SetValMeta(L);
        }
    }
    lua_call(L, n, LUA_MULTRET);						// ..., rtv...?

    auto sg = xx::MakeSimpleScopeGuard([&] {
        lua_settop(L, top);								// ...
        });
    auto top2 = lua_gettop(L);
    if (top2 > top) {                                   // has rtv
        xx_assert(top + 1 == top2);                     // only support 1 rtv
        ToVals(L, top + 1, 1);
        return *gValptrs[0];
    } else {
        return gNilVal;
    }
}

EMSCRIPTEN_BINDINGS(my_binds1) {
    emscripten::function("Callback", &Callback);
}


void Lua_Register_FromJS(lua_State* L) {
    xl::SetGlobalCClosure(L, "FromJS", [](auto L){
        auto name = xl::To<char const*>(L, 1);
        auto v = V::global(name);
        if (PushVal(L, v)) {
            new(lua_newuserdatauv(L, sizeof(V), 2)) V(std::move(v));
            SetValMeta(L);
        }
        return 1;
    });
}
