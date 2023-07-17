/*
emcc -lembind -o quick_example.js quick_example.cpp
*/
#include <emscripten/bind.h>

using namespace emscripten;

float lerp(float a, float b, float t) {
    return (1 - t) * a + t * b;
}

std::string hi(std::string name) {
    return "hi " + name;
}

struct Foo {
    std::string name;
    Foo(int n) {
        name = std::to_string(n);
    }
    void ClearName() { name.clear(); }
    std::string const& GetName() const { return name; }
    void SetName(std::string name) { this->name = std::move(name); }
};

EMSCRIPTEN_BINDINGS(my_module) {
    function("lerp", &lerp);
    function("hi", &hi);
    class_<Foo>("Foo")
            .constructor<int>()
            .function("ClearName", &Foo::ClearName)
            .property("name", &Foo::GetName, &Foo::SetName);
}
