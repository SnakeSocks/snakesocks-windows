# SnakeSocks client for Windows

## nogui edition

Here is skcli backend for win32. If you're interested in it, follow instructions below. Else, try gui edition in [release](https://github.com/SnakeSocks/snakesocks-windows/releases).

### Download from release

Needn't instruction, OK?

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

### Download from release

Needn't instruction, OK?

### Build from source

I build the project under Visual Studio 2017, so you need it with C#&WPF support. You need to compile nogui edition by

```sh
	cmake -G "MSYS Makefiles" -D CMAKE_BUILD_TYPE=Release -D FOR_GUI=1 . && make
	mv skcli.exe skcli-gui-backend.exe
```

OK. Now open `gui-visual-studio/SnakeSocksClientGUI.sln`, then build it in VS, then everything is fine. Enjoy!

### Usage

Select the path of skcli-gui-backend.exe. Fill all configurations. Enable proxy.

Tip: 'Autorun on boot' and 'Autoenable on run' is under building. Sorry and please wait...

I'll save your configurations in `./skgui.xml`.

## Compile your module for win32

Assuming that you're compiling my_module.c, put stdafx.h correctly and try:

```sh
	gcc -shared -o my_module.dll my_module.c
	# g++ is ok. Don't forget your 'extern "C"'.
```

Or maybe you're willing to link more obj to my_module.o... Everything is OK! 