ASM:=../../bin/asrv32im
SIM:=../../bin/simrv32im

ASM_SRC:=$(wildcard *.s)
OBJS:=$(patsubst %.s,%.o,$(ASM_SRC))
RUN:=$(patsubst %.o,%.run,$(OBJS))

all: $(RUN)
	@echo All tests ok

.PRECIOUS: %.o
%.o: %.s
	$(ASM) $< -o $@

.PHONY: %.run
%.run: %.o
	$(SIM) -x $<

.PHONY: clean
clean:
	rm $(OBJS)
