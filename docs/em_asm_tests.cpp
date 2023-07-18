/*
emcc em_asm_tests.cpp -lembind -Oz -o em_asm_tests.html

emrun em_asm_tests.html

show console:
enter browse console mode: ( chrome : Control-Shift-J ) ( firefox: Control-Shift-K )
*/
#include <emscripten/emscripten.h>
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

int main() {
    PrintStr("asd沃日f");
    auto obj = emscripten::val::object();
    obj.set("x", 1.2);
    obj.set("y", 3.4);
    PrintVal(obj);
    return 0;
}
