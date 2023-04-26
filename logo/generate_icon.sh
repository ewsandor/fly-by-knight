#!/bin/bash
inkscape -w 16   -h 16   -o flybyknight_16x16.png logo.svg
inkscape -w 24   -h 24   -o flybyknight_24x24.png logo.svg
inkscape -w 32   -h 32   -o flybyknight_32x32.png logo.svg
inkscape -w 48   -h 48   -o flybyknight_48x48.png logo.svg
inkscape -w 64   -h 64   -o flybyknight_64x64.png logo.svg
inkscape -w 128  -h 128  -o flybyknight_128x128.png logo.svg
inkscape -w 256  -h 256  -o flybyknight_256x256.png logo.svg
inkscape -w 512  -h 512  -o flybyknight_512x512.png logo.svg
inkscape -w 1024 -h 1024 -o flybyknight_1024x1024.png logo.svg

convert logo.svg -define icon:auto-resize=256,128,48,32,16 flybyknight.ico