#include "pch.h"
//using LuaStateWithInt = xx::Lua::StateWithExtra<int>;
//#define RefWeakTableRefId(L) LuaStateWithInt::Extra(L)
//#include <xx_lua_shared.h>
//LuaStateWithInt L;
//xl::MakeUserdataWeakTable(L);
//xl::RegisterBaseEnv(L);

void PrintVal(V const& v);
bool IsFunction(V const& v);

std::vector<V> gVals;
std::vector<V*> gValptrs;
std::unordered_map<std::string, double> gMap;
xl::State gL;
V gV;

// todo: cache metatable to registry
void FillGValptrs(lua_State* L, int n);
void SetValMeta(lua_State* L);
int HandleVal(lua_State* L, V& m);
int PushValFunction(lua_State* L, V& m);
int HandleVal(lua_State* L, V& m);
void SetValMeta(lua_State* L);

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
        } else if (IsFunction(m)) {
            PushValFunction(L, m);
        } else if (m.isArray()) {
            // todo?
            xx_assert(false);
        } else {
            new(lua_newuserdata(L, sizeof(V))) V(std::move(m));
            SetValMeta(L);
        }
    }
    lua_call(L, n, LUA_MULTRET);						// ..., rtv...?
    auto sg = xx::MakeSimpleScopeGuard([&]{
        lua_settop(L, top);								// ...
    });
    auto top2 = lua_gettop(L);
    if (top2 > top) {                                   // has rtv
        xx_assert(top + 1 == top2);
        auto t = lua_type(L, top2);
        switch(t) {
            case LUA_TNIL:
                gV = V::null();
                break;
            case LUA_TBOOLEAN:
                gV = V(xl::To<bool>(L, top2));
                break;
            case LUA_TLIGHTUSERDATA:
                xx_assert(false);
            case LUA_TNUMBER:
                gV = V(xl::To<double>(L, top2));
                break;
            case LUA_TSTRING:
                gV = V(xl::To<char const*>(L, top2));
                break;
            case LUA_TTABLE: {
                xl::To(L, top2, gMap);
                xx_assert(!gMap.empty());
                gV = V::object();
                for (auto &kv: gMap) {
                    gV.set(kv.first, kv.second);
                }
                gMap.clear();
                break;
            }
            case LUA_TFUNCTION: {
                auto k = luaL_ref(L, LUA_REGISTRYINDEX);
                auto s = std::string("CppCallback = (...args)=>{ return WasmCallback(" + std::to_string(k) + ", args); };");
                emscripten_run_script(s.c_str());   // todo: gc?
                gV = emscripten::val::global("CppCallback");
                break;
            }
            case LUA_TUSERDATA:
                return *(V*)lua_touserdata(L, top2);
            case LUA_TTHREAD:
                xx_assert(false);
            default:
                std::cout << "unsupported t ==" << t << std::endl;
                xx_assert(false);
        }
    } else {
        gV = V{};
    }
    return gV;
}
EMSCRIPTEN_BINDINGS(my_binds1) {
    emscripten::function("Callback", &Callback);
}

void FillGValptrs(lua_State* L, int n) {
    gVals.resize(n);
    gValptrs.resize(n);

    for(int i = 0; i < n; ++i) {
        auto idx = i + 1;
        auto t = lua_type(L, idx);
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
            default:
                std::cout << "unsupported t ==" << t << std::endl;
                xx_assert(false);
        }
    }
}


EM_JS(void, PrintValHandle, (emscripten::EM_VAL h), {
    console.log(Emval.toValue(h));
})

void PrintVal(V const& v) {
    PrintValHandle(v.as_handle());
}

EM_JS(bool, IsFunction_, (emscripten::EM_VAL h), {
    const v = Emval.toValue(h);
    if (typeof(v) === "function"){
        return !(/^class\\s/.test(Function.prototype.toString.call(v)));
    }
    return false;
})

bool IsFunction(V const& v) {
    return IsFunction_(v.as_handle());
}

int PushValFunction(lua_State* L, V& m) {
    lua_pushcclosure(L, [](lua_State *L) -> int {
            auto p = (V*)lua_touserdata(L, lua_upvalueindex(1));
            auto memberName = xl::To<char const *>(L, lua_upvalueindex(2));

            auto n = lua_gettop(L);
            if (n) {    // auto remove "self" args
                if (lua_type(L, 1) == LUA_TUSERDATA && lua_touserdata(L, 1) == (void *)p) {
                    lua_remove(L, 1);
                    --n;
                }
                FillGValptrs(L, n);
            }
            V r;
            switch (n) {
                case 0:
                    r = p->template call<V>(memberName);
                    break;
                case 1:
                    r = p->template call<V>(memberName, *gValptrs[0]);
                    break;
                case 2:
                    r = p->template call<V>(memberName, *gValptrs[0], *gValptrs[1]);
                    break;
                case 3:
                    r = p->template call<V>(memberName, *gValptrs[0], *gValptrs[1], *gValptrs[2]);
                    break;
                case 4:
                    r = p->template call<V>(memberName, *gValptrs[0], *gValptrs[1], *gValptrs[2], *gValptrs[3]);
                    break;
                case 5:
                    r = p->template call<V>(memberName, *gValptrs[0], *gValptrs[1], *gValptrs[2], *gValptrs[3], *gValptrs[4]);
                    break;
                    // known issue: add more ?
                default:
                    std::cout << "too many call args n == " << n << std::endl;
                    xx_assert(false);
            }
            gVals.clear();

            return HandleVal(L, r);
        }, 2);  // ...
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
        if (m.isNull() || m.isUndefined()) {
            // first letter : lower case / upper case compatible
            std::string s(memberName);
            if (auto c = s[0]; c >= 'a' && c <= 'z') {
                s[0] = (char)std::toupper(c);
            } else if (c >= 'A' && c <= 'Z') {
                s[0] = (char)std::tolower(c);
            } else return 0;
            m = (*p)[s.c_str()];
            if (m.isNull() || m.isUndefined()) return 0;
            lua_pop(L, 1);
            xl::Push(L, s);
        }

        return HandleVal(L, m);
    });

    xl::SetFieldCClosure(L, "__newindex", [](auto L)->int{
        auto p = (V*)lua_touserdata(L, 1);
        auto memberName = xl::To<char const*>(L, 2);
        auto t = lua_type(L, 3);
        switch(t) {
            case LUA_TNIL:
                p->set(memberName, V::null());
                break;
            case LUA_TBOOLEAN:
                p->set(memberName, xl::To<bool>(L, 3));
                break;
            case LUA_TLIGHTUSERDATA:
                xx_assert(false);
            case LUA_TNUMBER:
                p->set(memberName, xl::To<double>(L, 3));
                break;
            case LUA_TSTRING:
                p->set(memberName, xl::To<char const*>(L, 3));
                break;
            case LUA_TTABLE: {
                xl::To(L, 3, gMap);
                xx_assert(!gMap.empty());
                auto o = V::object();
                for (auto &kv: gMap) {
                    o.set(kv.first, kv.second);
                }
                gMap.clear();
                p->set(memberName, o);
                break;
            }
            case LUA_TFUNCTION: {
                auto k = luaL_ref(L, LUA_REGISTRYINDEX);
                auto s = std::string("CppCallback = (...args)=>{ return WasmCallback(" + std::to_string(k) + ", args); };");
                emscripten_run_script(s.c_str());   // todo: gc?
                p->set(memberName, emscripten::val::global("CppCallback"));
                break;
            }
            case LUA_TUSERDATA:
                p->set(memberName, *(V*)lua_touserdata(L, 3));
                break;
            case LUA_TTHREAD:
                xx_assert(false);
            default:
                std::cout << "unsupported t ==" << t << std::endl;
                xx_assert(false);
        }
        return 0;
    });

    lua_setmetatable(L, -2);                                    // ..., ud
    xx_assert(lua_gettop(L) == top);
}


void Lua_Register_FromJS(lua_State* L) {
    xl::SetGlobalCClosure(L, "FromJS", [](auto L){
        auto name = xl::To<char const*>(L, 1);
        auto v = V::global(name);
        if (IsFunction(v)) {
            new(lua_newuserdata(L, sizeof(V))) V(std::move(v)); // ..., nil, m
            lua_newtable(L);
            xx::Lua::SetFieldCClosure(L, "__gc", [](lua_State* L)->int {
                ((V*)lua_touserdata(L, 1))->~V();
                return 0;
            });
            lua_setmetatable(L, -2);
            lua_pushcclosure(L, [](lua_State*L)->int{
                auto p = (V*)lua_touserdata(L, lua_upvalueindex(1));
                auto n = lua_gettop(L);
                if (n) {
                    FillGValptrs(L, n);
                }
                V r;
                switch(n) {
                    case 0:
                        r = (*p)();
                        break;
                    case 1:
                        r = (*p)(*gValptrs[0]);
                        break;
                    case 2:
                        r = (*p)(*gValptrs[0], *gValptrs[1]);
                        break;
                    case 3:
                        r = (*p)(*gValptrs[0], *gValptrs[1], *gValptrs[2]);
                        break;
                    case 4:
                        r = (*p)(*gValptrs[0], *gValptrs[1], *gValptrs[2], *gValptrs[3]);
                        break;
                    case 5:
                        r = (*p)(*gValptrs[0], *gValptrs[1], *gValptrs[2], *gValptrs[3], *gValptrs[4]);
                        break;
                        // known issue: add more ?
                    default:
                        xx_assert(false);
                }
                gVals.clear();

                return HandleVal(L, r);
            }, 1);  // ..., nil
            return 1;
        }
        return HandleVal(L, v);
    });
}
