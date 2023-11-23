bindir = ../bin
project = $(bindir)/simrv32im
override CFLAGS += 
override LDFLAGS +=

objdir = ./obj
override CFLAGS += -I$(objdir) -I. -g --std=c99 -pedantic-errors -Wall -Wconversion

c_src = $(wildcard *.c)

c_objects = $(patsubst %, $(objdir)/%, $(c_src:.c=.o))
object = $(c_objects)
deps = $(object:.o=.d)

.PHONY: all clean

all: $(project)

-include $(deps)

$(project): $(object) $(bindir)
	$(CC) $(LDFLAGS) $(object) -o $@

$(objdir)/%.o: %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(object): | $(objdir)

$(objdir):
	mkdir -p $@

$(bindir):
	mkdir -p $@

check:
	$(MAKE) -C tests

clean:
	rm -rf $(objdir)
	rm -f $(project) $(project:=.exe)
