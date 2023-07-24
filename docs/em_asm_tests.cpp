/*
emcc em_asm_tests.cpp -lembind -Oz -o em_asm_tests.html

emrun em_asm_tests.html

show console:
enter browse console mode: ( chrome : Control-Shift-J ) ( firefox: Control-Shift-K )
*/
#include <emscripten/emscripten.h>
#include <emscripten/val.h>
#include <iostream>

EM_JS(void, PrintStr, (char const* s), {
    console.log(UTF8ToString(s));
});

EM_JS(void, PrintValHandle, (emscripten::EM_VAL h), {
    console.log(Emval.toValue(h));
});

void PrintVal(emscripten::val const& v) {
    PrintValHandle(v.as_handle());
}

EM_JS(emscripten::EM_VAL, ValHandleToString, (emscripten::EM_VAL h), {
    let o = Emval.toValue(h);
    let s = JSON.stringify(o);
    return Emval.toHandle(s);
});

std::string ToString(emscripten::val const& v) {
    auto h = ValHandleToString(v.as_handle());
    return emscripten::val::take_ownership(h).as<std::string>();
}

int main() {
    PrintStr("asd沃日f");
    auto obj = emscripten::val::object();
    obj.set("x", 1.2);
    obj.set("y", 3.4);
    PrintVal(obj);

    EM_ASM(
            o = {};
            o.a = 123;
    );
    auto v = emscripten::val::global("o");
    std::cout << ToString(v) << std::endl;

    return 0;
}
