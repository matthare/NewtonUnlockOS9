#!/usr/bin/env python3
"""Package the Retro68 build for copying to a Mac OS 9 machine.

    python3 tools/package.py            (after cmake --build build)

Writes to dist/:
  Newton Unlock.bin   MacBinary II -- open it with StuffIt Expander on the Mac
  Newton Unlock.dsk   800K HFS floppy image containing the app (for emulators,
                      or Disk Copy / a real floppy)

Retro68 leaves all Finder flags clear. Without the "has bundle" flag the OS 9
Finder ignores the app's BNDL resource and shows a generic icon, so this sets
it. It also gives the app a proper Finder name with a space.
"""
import os, shutil, subprocess, sys

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUILD = os.path.join(HERE, 'build', 'NewtonUnlock.bin')
DIST = os.path.join(HERE, 'dist')
NAME = 'Newton Unlock'
HFSUTILS = os.path.expanduser('~/Retro68-build/toolchain/bin')

HAS_BUNDLE_HIGH_BYTE = 0x20     # kHasBundle = 0x2000; MacBinary byte 73 is the flags' high byte


def crc16_xmodem(data):
    crc = 0
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = (crc << 1) ^ 0x1021 if crc & 0x8000 else crc << 1
            crc &= 0xFFFF
    return crc


def main():
    data = bytearray(open(BUILD, 'rb').read())
    if data[0] != 0 or crc16_xmodem(data[:124]) != int.from_bytes(data[124:126], 'big'):
        sys.exit(f'{BUILD}: not a MacBinary II file')
    name = NAME.encode('mac_roman')
    data[1] = len(name)
    data[2:65] = name.ljust(63, b'\0')
    data[73] |= HAS_BUNDLE_HIGH_BYTE
    data[124:126] = crc16_xmodem(data[:124]).to_bytes(2, 'big')

    os.makedirs(DIST, exist_ok=True)
    binpath = os.path.join(DIST, NAME + '.bin')
    with open(binpath, 'wb') as f:
        f.write(data)
    print('wrote', binpath)

    dsk = os.path.join(DIST, NAME + '.dsk')
    h = lambda tool, *args: subprocess.run([os.path.join(HFSUTILS, tool), *args], check=True,
                                           stdout=subprocess.DEVNULL)
    with open(dsk, 'wb') as f:
        f.truncate(800 * 1024)
    h('hformat', '-l', NAME, dsk)
    h('hmount', dsk)
    try:
        h('hcopy', '-m', binpath, ':')
    finally:
        h('humount')
    print('wrote', dsk)


if __name__ == '__main__':
    main()
