# Random development notes


## Crash handling

Teensy's relatively new crash handling mechanism will output the address of failure. To use it,

1. make sure you build with `-g` to output symbols
```
build_flags =
  -I../../src
  -std=c++17
  -Wall
  -g
```

2. with the address, run:

```
"C:\Users\carl\.platformio\packages\toolchain-gccarmnoneeabi\bin\arm-none-eabi-addr2line.exe" -e "C:\root\git\thenfour\Clarinoid\projects\BOMMANOID\.pio\build\teensy41\firmware.elf" 0x117bc
```



## Bloaty & memory use
Teensy will crash mysteriously when running out of memory. Before 1.54, it was pretty hard to get quick insight about mem usage. But now we can see memory.

RAM1 is getting stretched thin much of the time, leaving a couple dozen kb for stack.

Statically-allocated things like lookup tables go there. So Modcurve LUT, scale follower, static variables.

Also CODE gets copied to RAM1 for speed, and this is actually significant, probably because of the amount of templated code we use.

For this it's very useful to use "bloaty". It runs on *nix (ubuntu in my case).

```
git clone https://github.com/google/bloaty.git
npm install cmake
cd bloaty
git submodule update --init --recursive
make -j6
./bloaty bloaty
```
