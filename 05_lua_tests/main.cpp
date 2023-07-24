#include <xx_lua_data.h>
using LuaStateWithInt = xx::Lua::StateWithExtra<int>;
#define RefWeakTableRefId(L) LuaStateWithInt::Extra(L)
#include <xx_lua_shared.h>
namespace xl = xx::Lua;

int main() {
    LuaStateWithInt L;
    xl::MakeUserdataWeakTable(L);
    xl::RegisterBaseEnv(L);
    xl::DoFile(L, "/res/test1.lua");
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
