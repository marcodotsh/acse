ASM:=../../bin/asrv32im

ASM_SRC:=$(wildcard *.s)
OBJS:=$(patsubst %.s,%.o,$(ASM_SRC))
STDERRS:=$(patsubst %.s,%.stderr.txt,$(ASM_SRC))
STDOUTS:=$(patsubst %.s,%.stdout.txt,$(ASM_SRC))
OBJS_EXPECTED:=$(patsubst %.s,%.expected.o,$(ASM_SRC))
CHECK:=$(patsubst %.s,%.ck,$(ASM_SRC))

.PHONY: all
all: $(CHECK)
	@echo All tests ok

$(ASM):
	$(MAKE) -C ..

.PRECIOUS: %.o
%.o: %.s $(ASM)
	$(ASM) $< \
	  -o $(patsubst %.s,%.o,$<) \
	  1> $(patsubst %.s,%.stdout.txt,$<) \
	  2> $(patsubst %.s,%.stderr.txt,$<) || true

.PHONY: %.ck
%.ck: %.o
	@if [ -f $(patsubst %.ck,%.o,$@) ]; then \
	  [ -f $(patsubst %.ck,%.expected.o,$@) ] || ( echo Output produced but not expected ; exit 1 ); \
	else \
	  [ ! -f $(patsubst %.ck,%.expected.o,$@) ] || ( echo Output expected but not produced ; exit 1 ); \
	fi
	if [ -f $(patsubst %.ck,%.o,$@) ]; then diff $(patsubst %.ck,%.o,$@) $(patsubst %.ck,%.expected.o,$@); fi
	diff $(patsubst %.ck,%.stderr.txt,$@) $(patsubst %.ck,%.expected.stderr.txt,$@)
	diff $(patsubst %.ck,%.stdout.txt,$@) $(patsubst %.ck,%.expected.stdout.txt,$@)

.PHONY: expected
expected: $(OBJS_EXPECTED)

.PHONY: clean_expected
clean_expected:
	rm -f $(OBJS_EXPECTED)

.PRECIOUS: %.expected.o
%.expected.o: %.s $(ASM)
	$(ASM) $< \
	  -o $(patsubst %.s,%.expected.o,$<) \
	  1> $(patsubst %.s,%.expected.stdout.txt,$<) \
	  2> $(patsubst %.s,%.expected.stderr.txt,$<) || true

.PHONY: clean
clean:
	rm -f $(OBJS) $(STDERRS) $(STDOUTS)
