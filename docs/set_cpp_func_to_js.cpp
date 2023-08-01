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

// todo: replace int to shared ptr obj for destruct remove callback?

int callbackAutoKey{};
std::unordered_map<int, std::function<emscripten::val(emscripten::val const&)>> callbacks;
emscripten::val Callback(int key, emscripten::val const& a) {
    return callbacks[key](a);
}

void SetMemberCallback(emscripten::val v, std::string fn) {
    callbackAutoKey++;
    callbacks.insert(std::make_pair(callbackAutoKey, [k = callbackAutoKey](emscripten::val const& a)->emscripten::val {
        auto s = std::string("cpp: callback. key = ") + std::to_string(k) + "\n";
        PrintStr(s.c_str());
        //PrintVal(a);
        int r = 0;
        if (a.isArray()) {
            r = a["length"].as<int>();
            for (int i = 0; i < r; ++i) {
                PrintVal(a[i]);
            }
        }
        return emscripten::val{r};
    }));
    auto s = std::string("CppCallback = (...args)=>{ return Module.Callback(" + std::to_string(callbackAutoKey) + ", args); };");
    emscripten_run_script(s.c_str());
    v.set(fn.c_str(), emscripten::val::global("CppCallback"));
}

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("Callback", &Callback);
    emscripten::function("SetMemberCallback", &SetMemberCallback);
}
