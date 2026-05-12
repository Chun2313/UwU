# Tetris Game (Raylib + C++)

A classic Tetris game built with C++ and Raylib.

## Building

### macOS (Homebrew)

```bash
brew install raylib
g++ main.cpp -o tetris -I/opt/homebrew/include -L/opt/homebrew/lib -lraylib
./tetris
```

### Arch Linux

```bash
sudo pacman -S raylib
g++ main.cpp -o tetris -std=c++11 -lraylib
./tetris
```

### Windows (MSYS2 / MinGW)

1. Install [MSYS2](https://www.msys2.org/)
2. Open **MSYS2 UCRT64** terminal and run:
```bash
pacman -S mingw-w64-ucrt-x86_64-raylib mingw-w64-ucrt-x86_64-gcc
```
3. Build:
```bash
g++ main.cpp -o tetris.exe -std=c++11 -lraylib -lgdi32 -lwinmm
```
4. Run `tetris.exe`

### Windows (vcpkg)

```bash
vcpkg install raylib
g++ main.cpp -o tetris.exe -std=c++11 -I<vcpkg_include> -L<vcpkg_lib> -lraylib -lgdi32 -lwinmm
```

## Controls

- **A** — Move left
- **D** — Move right
- **X** — Soft drop
- **R** — Rotate piece
