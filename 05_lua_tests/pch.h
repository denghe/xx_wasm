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

void PrintVal(V const& v);
bool IsFunction(V const& v);

extern std::vector<V> gVals;
extern std::vector<V*> gValptrs;
extern std::unordered_map<std::string, double> gMap;
// todo: cache metatable to registry
void FillGValptrs(lua_State* L, int n);
void SetValMeta(lua_State* L);
int HandleVal(lua_State* L, V& m);
int PushValFunction(lua_State* L, V& m);
int HandleVal(lua_State* L, V& m);
void SetValMeta(lua_State* L);

void Lua_Register_FromJS(lua_State* L);
