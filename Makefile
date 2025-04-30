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

SHADERSRC = $(shell find . -name "shader.*")
SHADEROBJ = $(patsubst ./shader.%, %.spv, $(SHADERSRC))

CFLAGS += `sdl2-config --cflags` -I/usr/local/include
LDFLAGS += `sdl2-config --libs` -L/usr/local/lib -lm
LDFLAGS += -lglfw -lvulkan -ldl -lX11 -lXxf86vm -lXrandr -lXi
CFLAGS += -Wall -Wextra -pedantic -g
CFLAGS +=  -I./src/headers
COMPILE = $(CC) $(CFLAGS)
LINK = $(LD) $(LDFLAGS)

DEPFLAGS = -MMD
all: $(PROG) $(SHADEROBJ)

$(PROG): $(OBJ)
	$(LINK) $^ -o $@

%.o: %.c %.d ./Makefile
	$(COMPILE) -c $(DEPFLAGS) -o $@ $<
%.d: ;
.PHONY: clean

%.spv: shader.%
	glslc -o $@ $<

clean:
	@touch $(DEP) $(OBJ) $(PROG) $(SHADEROBJ)
	@rm -r $(DEP) $(OBJ) $(PROG) $(SHADEROBJ)
clean_all:
	@touch $(DEP) $(OBJ) $(IGN_DEP) $(IGN_OBJ) $(PROG) $(SHADEROBJ)
	@rm -r $(DEP) $(OBJ) $(IGN_DEP) $(IGN_OBJ) $(PROG) $(SHADEROBJ)

include $(DEP)
