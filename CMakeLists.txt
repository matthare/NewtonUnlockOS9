# Build with Retro68 (https://github.com/autc04/Retro68):
#   PowerPC (Mac OS 9):  cmake -B build-ppc -DCMAKE_TOOLCHAIN_FILE=<Retro68-build>/toolchain/powerpc-apple-macos/cmake/retroppc.toolchain.cmake
#   68k (System 7+):     cmake -B build-68k -DCMAKE_TOOLCHAIN_FILE=<Retro68-build>/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake
#   then:                cmake --build build-ppc
cmake_minimum_required(VERSION 3.10)
project(NewtonUnlock C)

add_application(NewtonUnlock
    CREATOR "NwUl"
    main.c
    unlockers.c
    games.c
    NewtonUnlock.r
)
