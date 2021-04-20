# trs80-pi
A collection of scripts and programs designed to simulate the user experience of a TRS-80 Model 100 via Raspberry Pi.

## Building

Uses features of the C++17 standard, so GCC 7 or greater recommended.

GCC version 9.1 and lower require an additional flag `-lstdc++fs` which the makefile *should* detect and apply if required.

The following command builds all applications:
```
make
```
