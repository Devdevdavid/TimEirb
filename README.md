# TimEirb
Simulation of a 32 bits Timer with SystemC and TML2.0

# Getting Started

## Prerequisites

+ SystemC 2.3.3

## Installing

Download the project and extract it.

Add a line to the makefile with your name in order specify the path of your SystemC installation
```
yourName: SYSTEMC := /Users/David/Documents/tmp/systemc-2.3.3
yourName: all
```

Use make to build the app with your name
```sh
$ make yourName
```

The makefile will launch the application but you can launch it directly from the terminal:
```sh
$ ./run.x
```
