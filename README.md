# tips & urls

https://emscripten.org/index.html

https://developer.mozilla.org/zh-CN/docs/WebAssembly/C_to_wasm

https://techkblog.com/fixed-sharedarraybuffer-is-not-defined/

https://developer.chrome.com/docs/devtools/wasm/

https://developer.chrome.com/blog/wasm-debugging-2020/

https://github.com/jspanchu/wasm-bits

https://github.com/deathkiller/jazz2-native

https://github.com/jamjarlabs/JamJar

demo at ???????????\emsdk\upstream\emscripten\test

# emsdk

emsdk with install & activate latest version

system env add ( copy from emsdk_env.bat ):

EMSDK          ???????????\emsdk
EMSDK_NODE     ???????????\emsdk\node\??????????????64bit\bin\node.exe
EMSDK_PYTHON   ???????????\emsdk\python\?????-nuget_64bit\python.exe
JAVA_HOME      ???????????\emsdk\java\?????_64bit

system env path add:

???????????\emsdk
???????????\emsdk\upstream\emscripten
???????????\emsdk\upstream\bin
???????????\emsdk\node\?????????64bit\bin

install dependency libraries through compilation:

em++ -sUSE_SDL=2 o.cpp -o o.html


# ide1
clion

path add for ninja64.exe:
C:\Program Files\JetBrains\CLion 20????????????????????????????\bin\ninja\win\x64

path add for mingw32-make.exe:
C:\Program Files\JetBrains\CLion 20????????????????????????????\bin\mingw\bin

config ( for every cmake project ):
File -- Settings -- Build, Execution, Deployment -- CMake -- Debug ( default mingw ) CMake options:

-DCMAKE_TOOLCHAIN_FILE=???????????\emsdk\upstream\emscripten\cmake\Modules\Platform\Emscripten.cmake

more: ( ignore vs build dir )
mouse right click on "dir", Mark Directory As -- Excluded

# build
click clion "hammer" icon

# browse
clion terminal window:
cd cmake-build-????????
emrun ?????????.html

# ide2 ( better for c++ because clion lag & keep use so many cpu )
vs22 with llvm installed

use cmake gui fill -T        ClangCL         generate .sln & open

# build & browse
in vs22, hot key : ctrl + `  open developer powershells:
mkdir ?????
cd ?????
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release                      = Debug | Release | RelWithDebInfo
emmake make -j24
emrun ?????????.html
