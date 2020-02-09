PROG =  hiworld
CC = gcc
OUT = bin

DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
$(shell mkdir -p $(OUT) >/dev/null)
#DEPFLAGS = $@ -MMD -MF $(DEPDIR)/$*.Td
DEPFLAGS = -MT $@ -MMD -MF $(DEPDIR)/$*.Td
FILENAMES = $(notdir $(wildcard src/*.c))
_OBJS = $(FILENAMES:%.c=$(OUT)/%.o)
_DEPS = $(_OBJS:%.o=$(OUT)/%.d)
_SRCS = $(FILENAMES:%.c=src/%.c)

CXXFLAGS += `sdl2-config --cflags` -I/usr/local/include
CXXFLAGS += -Ofast
#CXXFLAGS += -g
LDFLAGS += `sdl2-config --libs` -L/usr/local/lib -lm
#LDFLAGS += -lm
COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CXXFLAGS) -c
all: $(PROG)

$(PROG): $(_OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LDFLAGS) $^ -o $@
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

$(OUT)/%.o : src/%.c $(DEPDIR)/%.d
	$(COMPILE.c) $< -o $@
	$(POSTCOMPILE)
$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

clean:
	@touch $(PROG) && rm -r $(OUT) $(DEPDIR) $(PROG)
.PHONY: clean
#include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(_SRCS))))
include $(wildcard .d/*)
