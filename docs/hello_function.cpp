/*
compile:
em++ .\hello_function.cpp -o hello_function.html -sEXPORTED_FUNCTIONS="_int_sqrt,_make_rand" -Oz

browse:
emrun .\hello_function.html

test:
enter browse console mode: ( chrome : Control-Shift-J ) ( firefox: Control-Shift-K )
_int_sqrt(12)
_make_rand()
_make_rand()

*/
#include <math.h>

extern "C" {

int int_sqrt(int x) {
    return sqrt(x);
}

int make_rand() {
    return rand();
}

}
