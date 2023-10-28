PROG = test
CC = gcc
LD = $(CC)
C_FILES = $(shell find . -name "*.c")
IGNORES = $(shell find . -name "*.c" | grep "!IGNORE")
SRC = $(filter-out $(IGNORES),$(C_FILES))
OBJ = $(patsubst %.c,%.o,$(SRC))
DEP = $(patsubst %.c,%.d,$(SRC))
IGN_OBJ = $(patsubst %.c,%.o,$(IGNORES))
IGN_DEP = $(patsubst %.c,%.d,$(IGNORES))

CFLAGS += `sdl2-config --cflags` -I/usr/local/include
LDFLAGS += `sdl2-config --libs` -L/usr/local/lib -lm
LDFLAGS += -lglfw -lvulkan -ldl -lX11 -lXxf86vm -lXrandr -lXi
CFLAGS += -Wall -Wextra -pedantic -g
CFLAGS +=  -I./src/headers
COMPILE = $(CC) $(CFLAGS)
LINK = $(LD) $(LDFLAGS)

DEPFLAGS = -MMD
all: $(PROG)

$(PROG): $(OBJ)
	$(LINK) $^ -o $@

%.o: %.c %.d ./Makefile
	$(COMPILE) -c $(DEPFLAGS) -o $@ $<
%.d: ;
.PHONY: clean

clean:
	@touch $(DEP) $(OBJ) $(PROG)
	@rm -r $(DEP) $(OBJ) $(PROG)
clean_all:
	@touch $(DEP) $(OBJ) $(IGN_DEP) $(IGN_OBJ) $(PROG)
	@rm -r $(DEP) $(OBJ) $(IGN_DEP) $(IGN_OBJ) $(PROG)

include $(DEP)
