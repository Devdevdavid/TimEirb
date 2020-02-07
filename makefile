ifeq ($(OS),Windows_NT)
    CCFLAGS += -D WIN32
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
    	CC = g++
        CCFLAGS += -D LINUX
        LDLIBS += -L $(SYSTEMC)/lib-linux64 -Xlinker -Bstatic -lsystemc -Xlinker -Bdynamic -lm -pthread
    endif
    ifeq ($(UNAME_S),Darwin)
    	CC = g++-9
        CCFLAGS += -D OSX
        LDLIBS += -L $(SYSTEMC)/lib-macosx64 -lsystemc -lm -pthread
    endif
endif

CPPFLAGS = -I ./src -I $(SYSTEMC)/include -Wno-deprecated -O0
SYSTEMC := /to/be/defined/systemc-2.3.3
LDLIBS += -L.

SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:%.cpp=%.o)

PROGRAM = run.x

none:
    @echo You need to choose a target: make [name]

david: SYSTEMC := /Users/David/Documents/tmp/systemc-2.3.3
david: all

aurelien: SYSTEMC := /net/s/ddevant/lib/systemc-2.3.3
aurelien: all

guillaume: SYSTEMC := /net/s/ddevant/lib/systemc-2.3.3
guillaume: all

ophelie: SYSTEMC := /net/s/ddevant/lib/systemc-2.3.3
ophelie: all

quentin: SYSTEMC := /net/s/qgales/systemc-2.3.3
quentin: all

victor: SYSTEMC := /net/s/vtexier/Documents/3A/S10/modelisation_et_langage_systeme/systemc-2.3.3
victor: all

all: $(PROGRAM)
	@ echo -- STARTING APP --
	@ ./$(PROGRAM)

clean:
	-rm -f $(OBJS) $(PROGRAM)

$(PROGRAM):	$(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(PROGRAM)

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@
