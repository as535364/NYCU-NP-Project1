.PHONY: all clean
# variables
CXX = g++
CFLAGS = -g -std=c++17 -Wall
SOURCES := $(shell find src/ -name "*.cpp")
OBJS = npshell.o utils.o

# target
all: $(OBJS)
	$(CXX) -o npshell $^ $(CFLAGS)

$(OBJS): $(SOURCES)
	$(CXX) $(CFLAGS) -c $^

clean:
	@rm *.o npshell -f
