# Fly by Knight - Chess Engine 
Created by Ed Sandor.
September 2011 - 2012, 2020-2023.

https://git.sandorlaboratories.com/edward/fly-by-knight

## Overview

**REWRITE IN PROGRESS**

Fly by Knight is an amateur chess engine currently implementing the xboard chess protocol.  This chess engine leverages my Farewell to King C chess library.

Versions 0.x.x supported only the xboard chess engine protocol.
Versions 1.x.x+ is is a rewrite and is designed to support the xboard protocol with UCI in mind.  UCI protocol support may be added later after xboard is stable.

## Usage

### Dependencies
#### Operating Systems
Fly by Knight is mainly developed and tested on Linux compiled with GCC and CI jobs are maintained to verify compilation under Debian (x86_64/arm64), Ubuntu, and Fedora environments.  Dependencies are kept to a minimum so Fly by Knight may ideally be compiled on Windows (MinGW) or macOS, but these environments are not actively maintained.
#### Libraries
- Farewell to King Chess Library: https://git.sandorlaboratories.com/edward/farewell-to-king.
- POSIX Thread (pthread) Library.
- [zlib](https://web.archive.org/web/20230404152038/https://zlib.net/) Compression Library.

### Building and Running
```
// Inside the 'fly-by-knight' project folder
$ git submodule update --init --recursive
$ mkdir build
$ cd build
$ cmake ..
$ make
# make install
$ flybyknight
```

### GUI
Fly by Knight supports the xboard chess protocol compatible with many popular chess GUIs including [xboard](https://web.archive.org/web/20230402232639/https://www.gnu.org/software/xboard/).

## References
- [Chess Programming Wiki](https://www.chessprogramming.org)
- [UCI Protocol](https://web.archive.org/web/20230402232147/https://wbec-ridderkerk.nl/html/UCIProtocol.html)
- [xboard Protocol](https://web.archive.org/web/20230322184658/http://www.gnu.org/software/xboard/engine-intf.html)