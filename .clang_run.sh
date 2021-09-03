# Windows Clang build

set -e

echo "----- RUNNNING .clang_run.sh ---------"

# Add clang32 repo
echo "----- Adding clang32 repo to /etc/pacman.conf ---------"
echo "[clang32]" >> /etc/pacman.conf
echo "Include = /etc/pacman.d/mirrorlist.mingw" >> /etc/pacman.conf

echo "----- Installing pacman packages ---------"
pacman -Syu --noconfirm --needed -y base-devel clang32/mingw-w64-clang-i686-clang clang32/mingw-w64-clang-i686-compiler-rt make autoconf automake-wrapper

# Download and use prebuilt SDL2_mixer. This relies on the configure/make not clearing the src/extlib folder, if that behavior is changed this may not work.
echo "----- Copying prebuilt SDL32 mixer to src/extlib ---------"
curl -s https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-devel-2.0.0-mingw.tar.gz | tar xvz
cp -rf SDL2_mixer-2.0.0/i686-w64-mingw32/* src/extlib
rm -rf SDL2_mixer-2.0.0

# Use prebuilt SDL2_mixer, but also compile libvorbis/libogg so the engine can statically link to that. Prevents no audio / audio crash issue.
# Force usage of clang
echo "----- Starting build ---------"
export CC="clang"
export CXX="clang++"

# Enable adresss sanitizer
export CFLAGS="-fsanitize=address -fno-optimize-sibling-calls"
export CXXFLAGS="-fsanitize=address -fno-optimize-sibling-calls"
export LDFLAGS="-fsanitize=address -fno-optimize-sibling-calls"

# Disable Ponscripter's stdio redirect in SDL_win32_main.c to view asan's output on crash (asan doesn't seem to work with output redirection)
# Using CPPFLAGS doesn't seem to work. CFLAGSEXTRA will eventually be used in PSCFLAGS, which eventually gets passed to SDL_win32_main.c
export CFLAGSEXTRA="-DNO_STDIO_REDIRECT"

# this disables optimization and adds some debug flags(-O0)
export DEBUG=1

./configure $STEAM --unsupported-compiler --internal-all-mixers --disable-internal-sdl_mixer --force-external-sdl-mixer
make -j1
