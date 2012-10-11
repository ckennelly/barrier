CC=gcc
CXX=g++
RM=rm
WARNINGS := -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-align \
			-Wno-unused-parameter -Wwrite-strings -Wmissing-declarations \
			-Wredundant-decls -Winline -Wno-long-long -Wuninitialized \
			-Wconversion -Werror
CWARNINGS := $(WARNINGS) -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes
CXXWARNINGS := $(WARNINGS)
CFLAGS := -g -fPIC -std=gnu99 $(CWARNINGS)
CXXFLAGS := -g $(CXXWARNINGS)

SRCOBJS := $(patsubst %.cpp,%.o,$(wildcard *.cpp)) $(patsubst %.c,%.o,$(wildcard *.c))

all: test

test: $(SRCOBJS)
	gcc -o test $(SRCOBJS) -lpthread
	./test 1
	./test 2
	./test 4

# Blindly depend on all headers
%.o: %.c Makefile  $(wildcard *.h)
	$(CC) $(CFLAGS) -fPIC -MMD -MP -c $< -o $@

%.o: %.cpp Makefile $(wildcard *.h)
	$(CXX) $(CXXFLAGS) -fPIC -MMD -MP -c $< -o $@

clean:
	-$(RM) -f $(SRCOBJS) $(patsubst %.o,%.d,$(SRCOBJS)) test

.PHONY: all clean
