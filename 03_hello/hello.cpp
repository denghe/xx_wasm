#include <emscripten.h>

int main() {
    EM_ASM({
        console.log("hello, world!");
    });
    return 0;
}
