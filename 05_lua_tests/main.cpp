#include "pch.h"

int main() {
    emscripten_run_script(R"(


a = function() {
    console.log( "a" );
    this.abc = 123;
};
a.prototype.Dump = function() {
    console.log( this.abc );
}

aa = new a;
aa.Dump();
ff = aa.Dump;
ff.apply(aa)
console.log( "******************************" )


b = new Object;
c = { x:1, y:2 };
d = [ 1, 2 ];
e = 123;
console.log( typeof(a) )
console.log( typeof(b) )
console.log( typeof(c) )
console.log( typeof(d) )

var ks = Object.keys( a );
for( i = 0; i < ks.length; ++i ) {
    console.log( ks[i] + ':' + a[ks[i]] );
}


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

fff = ()=>{
    console.log( "js: global fff lambda called" )
};

)");

    xl::State L;
    Lua_Register_FromJS(L);

    xl::DoString(L, R"(
local c = FromJS( "c" )
local c2  = FromJS( "c" )

print( "lua: ", c )

local e = FromJS( "e" )
print( "lua: ", e )

local b = FromJS( "Bar" )
print( "lua: ", b )
local f =  b.Add
print( "lua: ", f )
print( "lua: ", f(1, 2) )

local o = b.CreateObject( "asdf" )
print( "lua: ", o )
b.Log( o )

local p = FromJS( "p" )
print( "lua: ", p.x )

local fff = FromJS( "fff" )
fff()

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
