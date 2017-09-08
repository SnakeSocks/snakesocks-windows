# SnakeSocks client for Windows

## Download from release

Get [release](https://github.com/SnakeSocks/snakesocks-windows/releases), then run skcli.exe or skgui.exe. 

## Build from source

### nogui edition

Build mingw project from source is **not** recommended, because of its annoying configurations. You need:

- Boost (Both headers and boost_system pre-compiled static library)

- Mingw-gcc && Mingw-g++

- MSYS Cmake and make

Usually it's necessary to edit CMakeLists_win.txt and replace CMakeLists.txt with it. Try launching your build:

```sh
git clone https://github.com/snakesocks/snakesocks.git && cd snakesocks/src/client
vim CMakeLists_win.txt # Set some env var.
mv CMakeLists_win.txt CMakeLists.txt
cmake -G "MSYS Makefiles" -D CMAKE_BUILD_TYPE=Release . && make
```

Then you'll get skcli.exe.

### gui edition

I build the project under Visual Studio 2017, so you need vs with C#&WPF support. 

After building skcli.exe, build `gui-visual-studio/SnakeSocksClientGUI.sln` in VS, then everything is OK. Enjoy!

## GUI usage

Select the path of skcli.exe. Fill all configurations. Enable proxy.

Tip: 'Autorun on boot' and 'Autoenable on run' is under building. Sorry and please wait...

I'll save your configurations in `./skgui.xml`.

## Compile your module for win32

Assuming that you're compiling my_module.c, put stdafx.h correctly and try:

```sh
	gcc -shared -o my_module.dll my_module.c
	# g++ is ok. Don't forget your 'extern "C"'.
```

Or maybe you're willing to link some other obj to my_module.o... Everything is OK! 
