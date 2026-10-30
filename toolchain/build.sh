#!/usr/bin/env bash
# toolchain/build.sh: build the cross-compiler (C and C++) for Charon
set -euo pipefail

# ---- pinned versions (bump deliberately, then re-test) ----
BINUTILS_VER="2.43.1"
GCC_VER="14.2.0"

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC="$ROOT/src"
BUILD="$ROOT/build"
PREFIX="${PREFIX:-$HOME/opt/cross}"
JOBS="$(nproc)"

# Host-side C++ flags. A new host GCC defaults to C++20, which breaks
# GCC 14.2's bundled libcody (u8"" literals became char8_t).
HOST_CXXFLAGS="-g -O2 -std=gnu++11"

# ---- ask: target ----
echo "Which target?"
echo "  1) i686-elf    (32-bit, matches the current kernel)"
echo "  2) x86_64-elf  (64-bit)"
read -rp "Choice [1]: " t
case "${t:-1}" in
  1) TARGET="i686-elf" ;;
  2) TARGET="x86_64-elf" ;;
  *) echo "Invalid choice"; exit 1 ;;
esac

echo
echo "Will build binutils $BINUTILS_VER + gcc $GCC_VER for $TARGET into $PREFIX"
read -rp "Continue? [y/N]: " ok
[[ "${ok:-}" =~ ^[Yy]$ ]] || exit 0

# ---- host dependencies ----
missing=0
for cmd in gcc g++ make bison flex curl tar makeinfo; do
  command -v "$cmd" >/dev/null || { echo "Missing tool: $cmd"; missing=1; }
done
if command -v pacman >/dev/null; then
  for p in gmp mpfr libmpc texinfo; do
    pacman -Qq "$p" >/dev/null 2>&1 || { echo "Missing package: $p (sudo pacman -S $p)"; missing=1; }
  done
fi
[[ $missing -eq 0 ]] || { echo "Install the missing items (sudo pacman -S base-devel gmp mpfr libmpc texinfo) and rerun."; exit 1; }

mkdir -p "$SRC" "$BUILD" "$PREFIX"
export PATH="$PREFIX/bin:$PATH"

fetch() { # url file
  [[ -f "$SRC/$2" ]] || curl -fL "$1" -o "$SRC/$2"
  [[ -d "$SRC/${2%.tar.xz}" ]] || tar -xf "$SRC/$2" -C "$SRC"
}

fetch "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VER.tar.xz" "binutils-$BINUTILS_VER.tar.xz"
fetch "https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VER/gcc-$GCC_VER.tar.xz"   "gcc-$GCC_VER.tar.xz"

# ---- 1) binutils (assembler, linker, objdump...) ----
if command -v "$TARGET-ld" >/dev/null; then
  echo ">> binutils already installed, skipping"
else
  rm -rf "$BUILD/binutils" && mkdir -p "$BUILD/binutils" && cd "$BUILD/binutils"
  "$SRC/binutils-$BINUTILS_VER/configure" \
    --target="$TARGET" --prefix="$PREFIX" \
    --with-sysroot --disable-nls --disable-werror
  make -j"$JOBS"
  make install
fi

# ---- 2) gcc (C and C++ compilers + libgcc; no libc exists yet) ----
rm -rf "$BUILD/gcc" && mkdir -p "$BUILD/gcc" && cd "$BUILD/gcc"
CXXFLAGS="$HOST_CXXFLAGS" "$SRC/gcc-$GCC_VER/configure" \
  --target="$TARGET" --prefix="$PREFIX" \
  --disable-nls --enable-languages=c,c++ --without-headers

make -j"$JOBS" CXXFLAGS="$HOST_CXXFLAGS" all-gcc 2>&1 | tee gcc-build.log
make -j"$JOBS" all-target-libgcc 2>&1 | tee libgcc-build.log
make CXXFLAGS="$HOST_CXXFLAGS" install-gcc
make install-target-libgcc

# ---- verify ----
echo
for tool in gcc g++; do
  command -v "$TARGET-$tool" >/dev/null \
    || { echo "ERROR: $TARGET-$tool was not installed"; exit 1; }
done
"$TARGET-g++" --version | head -1

echo
echo "Done. Add this to your shell profile (~/.bashrc or ~/.zshrc):"
echo "  export PATH=\"$PREFIX/bin:\$PATH\""
