# Fly by Knight - Chess Engine 
Created by Ed Sandor.
September 2011 - 2012, 2020-2021.

## Overview

**REWRITE IN PROGRESS**

Fly by Knight is an amateur chess engine currently implementing the xboard chess protocol.  This chess engine leverages my Farewell to King C chess library.

Versions 0.x.x supported only the xboard chess engine protocol.
Versions 1.x.x+ is is a rewrite and is designed to support the xboard protocol.  UCI protocol support may be added later after xboard is stable.

## Usage

### Dependencies
- Farewell to King Chess Library: https://git.sandorlabs.us/edward/farewell-to-king.
- POSIX for threading.

### Building
```
$ cd fly-by-knight
$ mkdir build
$ cd build
$ cmake ..
$ make
# make install
```

### Running
```
$ cd fly-by-knight/build/src/
$ ./fly_by_knight
```

### GUI
Fly by Knight supports the xboard chess protocol compatible with many popular chess GUIs including xboard: https://www.gnu.org/software/xboard/.

## References
xboard Protocol:
https://www.gnu.org/software/xboard/engine-intf.html
UCI Protocol:
http://wbec-ridderkerk.nl/html/UCIProtocol.html
