cd cmake-build-release
copy /y hello_world.wasm bak.wasm
llvm-strip.exe --strip-all bak.wasm -o hello_world.wasm