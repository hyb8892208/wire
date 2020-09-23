all: Emu_test probe_emu

CC       ?= gcc
CFLAGS   ?= -Wall -g

COBJS     = serial.o Emu_test.o
OBJS      = $(COBJS) 
PROBE_EMU_OBJS = serial.o probe_emu.o

Emu_test: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

probe_emu:$(PROBE_EMU_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $(INCLUDES) $< -o $@

clean:
	rm -rf $(OBJS) Emu_test

.PHONY: clean
