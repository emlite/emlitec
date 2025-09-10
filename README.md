# Emlitec
Emlite is a tiny JS bridge for native C/C++ code via Wasm, which is agnostic of the underlying toolchain. Thus it can target wasm32-unknown-unknown (freestanding, via stock clang), wasm32-wasi, wasm32-wasip1 and emscripten. 
It allows plain C or C++ code — compiled for wasm — to interoperate with javascript (including the DOM) and other JavaScript objects/runtimes without writing much JS “glue.”
It provides both a C api and a higher level C++ api similar to emscripten's val api.
For freestanding builds, it provides a simple bump allocator (invocable via malloc), however this repo also vendors dlmalloc in the src directory. Please check the CMakeLists.txt to see how it's used in the tests and examples.

## Requirements
To use the C api, a C11 capable compiler should be sufficient.

## Since emscripten exists, why would I want to use wasm32-wasi or wasm32-wasip1?
- Emscripten is a large install (around 1.4 gb), and bundles clang, python, node and java.
- In contrast, if you already have clang installed, wasi-libc's sysroot is around 2.4mb if you're only using Emlite's C api, or C++ with only C headers (nostdlib++).
- The wasi-sysroot/wasm32-wasi is only 44mb (headers and libraries) which includes C++ std headers and libs.
- Even if you install the wasi-sdk, it still is less than 1/4 the size of emscripten.
- Emscripten javascript glue is sometimes difficult to navigate when a problem occurs.

## Why emscripten instead of wasm32-wasi
- More established.
- Offers other bundled libraries like SDL, boost and other ports.
- Offers Asyncify and emscripten_sleep out of the box which allows better compatibilty for compiling sources containing event-loops like games.
- Automatic translation of OpenGL to WebGL.
- Offers more optimisations by bundling binaryen, wasm-opt and google's Closure compiler.

## Examples
```c
#include <emlite/emlite.h>

int main() {
    emlite_init_handle_table();
    em_Val console = em_Val_global("console");
    em_Val_call(console, "log", 1, em_Val_from_string("200"));
    emlite_reset_object_map();
}
```

To quickly try out emlite in the browser, create an index.html file:
(Note this is not the recommended way to deploy. You should install the required dependencies via npm and use a bundler like webpack to handle bundling, minifying, tree-shaking ...etc).

- Using wasm32-wasi[p1] (wasi-libc, wasi-sysroot, wasi-sdk or standalone_wasm emscripten):
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
</head>
<body>
    <script type="module">
        import { WASI, File, OpenFile, ConsoleStdout } from "https://unpkg.com/@bjorn3/browser_wasi_shim";
        import { Emlite } from "https://unpkg.com/emlite";
        // or (if you decide to vendor emlite.js)
        // import { Emlite } from "./src/emlite.js";

        window.onload = async () => {
            let fds = [
                new OpenFile(new File([])), // 0, stdin
                ConsoleStdout.lineBuffered(msg => console.log(`[WASI stdout] ${msg}`)), // 1, stdout
                ConsoleStdout.lineBuffered(msg => console.warn(`[WASI stderr] ${msg}`)), // 2, stderr
            ];
            let wasi = new WASI([], [], fds);
            const emlite = new Emlite();
            const bytes = await emlite.readFile(new URL("./bin/mywasm.wasm", import.meta.url));
            let wasm = await WebAssembly.compile(bytes);
            let inst = await WebAssembly.instantiate(wasm, {
                "wasi_snapshot_preview1": wasi.wasiImport,
                "env": emlite.env,
            });
            emlite.setExports(inst.exports);
            // if your C/C++ has a main function, use: `wasi.start(inst)`. If not, use `wasi.initialize(inst)`.
            wasi.start(inst);
        };
    </script>
</body>
</html>
```

- Freestanding
The @bjorn3/browser_wasi_shim dependency is not required for freestanding builds:
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
</head>
<body>
    <script type="module">
        import { Emlite } from "https://unpkg.com/emlite";
        // or (if you decide to vendor emlite.js)
        // import { Emlite } from "./src/emlite.js";

        window.onload = async () => {
            const emlite = new Emlite();
            const bytes = await emlite.readFile(new URL("./bin/mywasm.wasm", import.meta.url));
            let wasm = await WebAssembly.compile(bytes);
            let inst = await WebAssembly.instantiate(wasm, {
                "env": emlite.env,
            });
            emlite.setExports(inst.exports);
            inst.exports.main?.();
            window.alert(inst.exports.add?.(1, 2));
        };
    </script>
</body>
</html>
```

## Building
### Using CMake
You can use CMake's FetchContent to get this repo, otherwise you can just copy the header files into your project.

To build with the wasi-sdk or emscripten, it's sufficient to pass the necessary toolchain file:
```bash
# You would have to set $EMSCRIPTEN_ROOT or $WASI_SDK accordingly
cmake -Bbin -GNinja -DCMAKE_TOOLCHAIN_FILE=$WASI_SDK/share/cmake/wasi-sdk.cmake && cmake --build bin
# or
cmake -Bbin -GNinja -DCMAKE_TOOLCHAIN_FILE=$EMSCRIPTEN_ROOT/cmake/Modules/Platform/Emscripten.cmake && cmake --build bin
```

To build using cmake for freestanding or with wasi-libc or wasi-sysroot, it's preferable to create a cmake toolchain file and pass that to your invocation:
```bash
cmake -Bbin -GNinja -DCMAKE_TOOLCHAIN_FILE=./my_toolchain_file.cmake
```

The contents of your toolchain file should be adjust according to your needs. Please check the cmake directory of this repo for examples.

Note that there are certain flags which must be passed to wasm-ld in your CMakeLists.txt file:
```cmake
set_target_properties(mytarget PROPERTIES LINKER_LANGUAGE CXX SUFFIX .wasm LINK_FLAGS "-Wl,--no-entry,--allow-undefined,--export-dynamic,--export-if-defined=main,--export-table,--import-memory,--export-memory,--strip-all")
```
Also check the CMakeLists.txt file in the repo to see how the examples and tests are built.

### Using clang bundled with wasi-sdk
- No need to pass a sysroot, nor a target:
```bash
clang -Iinclude -o my.wasm main.c -Wl,--no-entry,--allow-undefined,--export-dynamic,--export-if-defined=main,--export-table,--import-memory,--export-memory,--strip-all
```

### Using stock clang
clang capable of targeting wasm32 is required. 
If you installed your clang via a package manager, you might require an extra package like libclang-rt-dev-wasm32 (note that it should match the version of your clang install, i.e libclang-rt-18-dev-wasm32). 
Additionally you might require lld to get wasm-ld. Similarly, it should match your clang version.

#### Targeting wasi
- If only using C, you can get the wasi-libc sources from the [wasi-libc](https://github.com/WebAssembly/wasi-libc) repo (which requires compiling the source using the given instructions). There are also packages for debian/ubuntu, arch linux, and msys2.
- If using C++ as well, you can grab the wasi-sysroot from the [wasi-sdk](https://github.com/WebAssembly/wasi-sdk/releases) releases page.


#### Building for a wasm32-unknown-unknown
You can also pass wasm32 as the target, which clang will understand as wasm32-unknown-unknown.
As mentioned previously, emlite only includes a simple bump allocator. It's advisable to utilise something like dlmalloc (vendored in the source directory).

## Testing
To test emlite, you can clone this repo and run it's test suite:
```bash
git clone https://github.com/emlite/emlitec --recurse-submodules
cd emlitec
npm install
npm run test_all
npm run serve
```

Building the tests requires CMake and Ninja.

test_all runs build_tests which builds by default for freestanding.
It will also build for wasi-libc, wasi-sysroot, wasi-sdk, and emscripten if the necessary environment variables are set:
- WASI_LIBC
- WASI_SYSROOT
- WASI_SDK
- EMSCRIPTEN_ROOT

It also runs gen_html_tests which genererates the necessary javascript glue code, runs webpack and creates the html files for testing. Each build directory should have an index.html file which has links to the rest of the html files.
Running wasm code requires starting a server, which can be done using npm run serve.
