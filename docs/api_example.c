/*
emcc api_example.c -o api_example.js -sMODULARIZE -sEXPORTED_RUNTIME_METHODS=ccall
*/
#include <stdio.h>
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
void sayHi() {
    printf("Hi!\n");
}

EMSCRIPTEN_KEEPALIVE
int daysInWeek() {
    return 7;
}
