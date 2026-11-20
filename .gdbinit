# Mark ESP32-C3 XIP Flash & DROM as Read-Only so GDB forces hardware breakpoints
mem 0x42000000 0x43000000 ro
mem 0x3C000000 0x3C800000 ro
mem 0x3FC80000 0x3FCE0000 rw
set breakpoint auto-hw on