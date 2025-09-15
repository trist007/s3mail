#!/bin/bash

# Common compiler flags (macOS equivalents of your Windows flags)
CommonCompilerFlags="-std=c99 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-missing-field-initializers -O0 -g -ffast-math -fno-exceptions"

# Common linker flags
CommonLinkerFlags="-framework Carbon -framework OpenGL -framework ApplicationServices -framework CoreFoundation"

# Create build directory if it doesn't exist
if [ ! -d "../../build" ]; then
    mkdir -p ../../build
fi

pushd ../../build > /dev/null

# Clean previous builds
echo "Cleaning previous builds..."
rm -f *.dylib *.a *.o *.dSYM 2>/dev/null

# Build static library for graphics functions
echo "Building s3mail_graphics.a..."
clang $CommonCompilerFlags -c ../s3mail/code/s3mail.cpp -o s3mail_graphics.o
ar rcs s3mail_graphics.a s3mail_graphics.o

# Create lock file (equivalent to Windows PDB lock)
echo "WAITING FOR DYLIB" > lock.tmp

echo "Building libs3mail_game.dylib..."
# Build the game code as a dynamic library
clang $CommonCompilerFlags \
    -shared \
    -fPIC \
    -undefined dynamic_lookup \
    ../s3mail/code/s3mail_game.cpp \
    s3mail_graphics.a \
    -o libs3mail_game.dylib \
    $CommonLinkerFlags

# Remove lock file
rm -f lock.tmp

echo "Building s3mail executable..."
# Build the main executable
clang $CommonCompilerFlags \
    ../s3mail/code/macos_s3mail.c \
    s3mail_graphics.a \
    -o s3mail \
    $CommonLinkerFlags

# Make executable
chmod +x s3mail

echo "Build complete!"
echo "Executable: s3mail"
echo "Game library: libs3mail_game.dylib"
echo "Graphics library: s3mail_graphics.a"

popd > /dev/null
