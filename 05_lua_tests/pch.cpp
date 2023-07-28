#include "pch.h"

std::vector<V> gVals;
std::vector<V*> gValptrs;
std::unordered_map<std::string, double> gMap;

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

    lua_pushcclosure(L, [](lua_State*L)->int{
        auto& p = *(V*)lua_touserdata(L, lua_upvalueindex(1));
        auto memberName = xl::To<char const*>(L, lua_upvalueindex(2));

        auto n = lua_gettop(L);
        if (n) {
            // auto remove self ?
            if (lua_type(L, 1) == LUA_TUSERDATA && lua_touserdata(L, 1) == (void*)&p) {
                lua_remove(L, 1);
            }

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
                    case LUA_TFUNCTION:
                        // todo: emscripten_run_script create global function with unique name, then val get it ? how to gc ?
                        xx_assert(false);
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

        V r;
        switch(n) {
            case 0:
                r = p.template call<V>(memberName);
                break;
            case 1:
                r = p.template call<V>(memberName, *gValptrs[0]);
                break;
            case 2:
                r = p.template call<V>(memberName, *gValptrs[0], *gValptrs[1]);
                break;
            case 3:
                r = p.template call<V>(memberName, *gValptrs[0], *gValptrs[1], *gValptrs[2]);
                break;
            case 4:
                r = p.template call<V>(memberName, *gValptrs[0], *gValptrs[1], *gValptrs[2], *gValptrs[3]);
                break;
            case 5:
                r = p.template call<V>(memberName, *gValptrs[0], *gValptrs[1], *gValptrs[2], *gValptrs[3], *gValptrs[4]);
                break;
                // known issue: add more ?
            default:
                xx_assert(false);
        }
        gVals.clear();

        return HandleVal(L, r);
    }, 2);

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

    // todo: __newindex ?

    lua_setmetatable(L, -2);                                    // ..., ud
    xx_assert(lua_gettop(L) == top);
}
