/*
emcc -lembind -o set_cpp_func_to_js.js set_cpp_func_to_js.cpp

emrun set_cpp_func_to_js.html

show console:
enter browse console mode: ( chrome : Control-Shift-J ) ( firefox: Control-Shift-K )

*/

#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

EM_JS(void, PrintStr, (char const* s), {
    console.log(UTF8ToString(s));
});

EM_JS(void, PrintValHandle, (emscripten::EM_VAL h), {
    console.log(Emval.toValue(h));
});

void PrintVal(emscripten::val const& v) {
    PrintValHandle(v.as_handle());
}


int callbackAutoKey{};
std::unordered_map<int, std::function<void()>> callbacks;
void Callback(int key) {
    callbacks[key]();
}

void SetMemberCallback(emscripten::val v, std::string prop) {
    callbackAutoKey++;
    callbacks.insert(std::make_pair(callbackAutoKey, [k = callbackAutoKey]{
        PrintStr((std::string("call back. key = ") + std::to_string(k)).c_str());
    }));
    auto s = std::string("CppCallback = ()=>{ Module.Callback(" + std::to_string(callbackAutoKey) + "); };");
    emscripten_run_script(s.c_str());
    v.set(prop.c_str(), emscripten::val::global("CppCallback"));
}


EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("Callback", &Callback);
    emscripten::function("SetMemberCallback", &SetMemberCallback);
}
