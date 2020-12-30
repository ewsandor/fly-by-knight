# Fly by Knight - Chess Engine 
Created by Ed Sandor.
September 2011 - 2012, 2020.

## Overview

**REWRITE IN PROGRESS**

Fly by Knight is an amateur chess engine currently implementing the UCI chess protocol.  This chess engine leverages my Farewell to King C chess library.

## Usage

### Dependencies
- Farewell to King Chess Library: https://git.sandorlabs.us/edward/farewell-to-king.
- POSIX for Threading.

### Building
```
$ cd fly-by-knight
$ mkdir build
$ cd build
$ cmake ..
$ make
```

### Running
```
$ cd fly-by-knight/build/src/
$ ./fly_by_knight
```

### GUI
Fly by Knight supports the UCI chess protocol compatible with many popular chess GUIs including XBoard: https://www.gnu.org/software/xboard/.

## References
UCI Protocol:
http://wbec-ridderkerk.nl/html/UCIProtocol.html
