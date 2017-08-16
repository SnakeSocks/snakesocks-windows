# SnakeSocks client for Windows

## nogui edition

Here is skcli backend for win32. If you're interested to it, follow instructions below. Else, try gui edition in [release](https://example.com).

### Build from source

Build mingw project from windows is not recommended, because of its annoying configurations. You need:

- Boost (Both headers and boost_system pre-compiled static library)

- Mingw-gcc && Mingw-g++

- MSYS Cmake and make

You may need to edit CMakeLists.txt to have all of them linked correctly. Then compile it: 

```sh
    cmake -G "MSYS Makefiles" -D CMAKE_BUILD_TYPE=Release . && make
```

Then enjoy skcli.exe.

## gui edition

[TODO]

## Compile your module for win32

Assuming that you're compiling my_module.c, put stdafx.h correctly and try:

```sh
	gcc -shared -o my_module.dll my_module.c
	# g++ is ok. Don't forget your 'extern "C"'.
```

Or maybe you're willing to link more obj to my_module.o... Everything is OK! 