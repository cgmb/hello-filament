# GLFW Hello Triangle

This is kinda hacked together, but it's a hello triangle for Google Filament
with GLFW.

Target platform is Ubuntu 16.04. This was tested against filament
a831c507cfd0e3fdce3e78df51fe5b7995f0b89c, with the build files modified
slightly so it could compile.

# Build Command

    CC=/usr/bin/clang-6.0 CXX=/usr/bin/clang++-6.0 cmake -H. -Bbuild && cmake --build build

