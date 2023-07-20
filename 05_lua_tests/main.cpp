#include <xx_lua_data.h>
using LuaStateWithInt = xx::Lua::StateWithExtra<int>;
#define RefWeakTableRefId(L) LuaStateWithInt::Extra(L)
#include <xx_lua_shared.h>
namespace xl = xx::Lua;

int main() {
    LuaStateWithInt L;
    xl::MakeUserdataWeakTable(L);
    xl::RegisterBaseEnv(L);
    xl::DoString(L, R"(

local t = {}
for j = 1, 100000 do
    -- todo
end
print(t)

)");
    return 0;
}
