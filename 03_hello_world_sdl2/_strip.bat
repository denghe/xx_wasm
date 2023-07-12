cd cmake-build-release
copy /y hello.wasm bak.wasm
llvm-strip.exe --strip-all bak.wasm -o hello.wasm