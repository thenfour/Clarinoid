# Random development notes


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

## Examining static memory usage

Best to just analyze the elf itself.

```
"C:\Users\carl\.platformio\packages\toolchain-gccarmnoneeabi-teensy\bin\arm-none-eabi-nm.exe" --print-size --size-sort --radix=d "C:\root\git\thenfour\Clarinoid\projects\CLARINOID2\.pio\build\teensy\firmware.elf"
```

https://www.pjrc.com/store/teensy40.html


## Addr2Line

I never managed to get it to work.

