PROG = test
CC = gcc
LD = gcc
#MAKEDEPEND = gcc -E
SRC = $(shell find . -name "*.c")
OBJ = $(patsubst %.c,%.o,$(SRC))
DEP = $(patsubst %.c,%.d,$(SRC))
CFLAGS = -Wall  `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs` -L/usr/local/lib
COMPILE = $(CC) $(CFLAGS)
LINK = $(LD) $(LDFLAGS)

DEPFLAGS = -MMD
all: $(PROG)

$(PROG): $(OBJ)
	$(LINK) $^ -o $@

%.o: %.c %.d
	$(COMPILE) -c $(DEPFLAGS) -o $@ $<
%.d: ;

.PHONY: clean

clean:
	@touch $(DEP) $(OBJ) $(PROG)
	@rm -r $(DEP) $(OBJ) $(PROG)
include $(DEP)
