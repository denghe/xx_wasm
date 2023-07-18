/*
emcc -lembind -o quick_example.js quick_example.cpp

emrun quick_example.html

show console:
enter browse console mode: ( chrome : Control-Shift-J ) ( firefox: Control-Shift-K )

*/

#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;

float lerp(float a, float b, float t) {
    return (1 - t) * a + t * b;
}

std::string hi(std::string name) {
    return "hi " + name;
}

struct Foo {
    std::string name;
    emscripten::val cb;
    Foo(int n) {
        name = std::to_string(n);
    }
    void ClearName() { name.clear(); }
    std::string const& GetName() const { return name; }
    void SetName(std::string name) { this->name = std::move(name); }
    void SetCB(emscripten::val cb_) {
        this->cb = std::move(cb_);
    }
    void CallCB() {
        this->cb();
    }
};

EMSCRIPTEN_BINDINGS(my_module) {
    function("lerp", &lerp);
    function("hi", &hi);
    class_<Foo>("Foo")
            .constructor<int>()
            .function("ClearName", &Foo::ClearName)
            .function("SetCB", &Foo::SetCB)
            .function("CallCB", &Foo::CallCB)
            .property("name", &Foo::GetName, &Foo::SetName);
}
