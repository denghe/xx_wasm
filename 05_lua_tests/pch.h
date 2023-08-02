#include <xx_lua_data.h>
namespace xl = xx::Lua;
extern xl::State gL;

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
using V = emscripten::val;

void Lua_Register_FromJS(lua_State* L);
