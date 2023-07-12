cd cmake-build-release
copy /y hello_world_sdl.wasm bak.wasm
llvm-strip.exe --strip-all bak.wasm -o hello_world_sdl.wasm