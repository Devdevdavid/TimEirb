ifeq ($(OS),Windows_NT)
    CCFLAGS += -D WIN32
    SYSTEMC=/to/update/systemc-2.3.3
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CCFLAGS += -D LINUX
        SYSTEMC=/net/s/ddevant/lib/systemc-2.3.3
        LDLIBS += -L $(SYSTEMC)/lib-linux64 -Xlinker -Bstatic -lsystemc -Xlinker -Bdynamic -lm -pthread
    endif
    ifeq ($(UNAME_S),Darwin)
        CCFLAGS += -D OSX
        SYSTEMC = /Users/David/Documents/tmp/systemc-2.3.3
        LDLIBS += -L $(SYSTEMC)/lib-macosx64 -lsystemc -lm -pthread
    endif
endif

CC = g++
CPPFLAGS = -I ./src -I $(SYSTEMC)/include -Wno-deprecated -O0

LDLIBS += -L.

SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:%.cpp=%.o)

PROGRAM = run.x

all: $(PROGRAM)
	@ echo -- STARTING APP --
	@ ./$(PROGRAM)

clean:
	-rm -f $(OBJS) $(PROGRAM)

$(PROGRAM):	$(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(PROGRAM)

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@
