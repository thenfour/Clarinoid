00 - 20220612 first work of a clarinoid3 test board. maybe biting off more than i can chew though.
01 - 20220612 let's break up that board into multiple sections
    20220618 after some work, this thing is pretty much the MEGA tester with tons of stuff.
              while that's nice work and nice thought, i think it's too risky for 1st try.
              let's simplify then come back to this.
02 - 20220618 - let's simplify back?
     eh, problem is i actually want to test all that stuff in 1 go. let's just make sub-boards.
03 - 20220618 copied from 01. let's just separate certain modules into sub-modules
04 - LED drivers are actually a nightmare and they're not really essential to this project. forget about them.
     need to separate:
     x dac (5)
     - midi
     - would be nice to have a way to switch between battery  or usb power

     20220620: 04 is being produced by jlcpcb

05 - now using teensy 4.1 with pins. ready for assembly.
     and even more modular design

     this was rejected; added 05-asm and 05-noasm to separate assembly versus non-assembly boards in hopes
     of cost savings and build simplification.

06 - that still wasn't enough; need to break yet into <=6 designs per board.