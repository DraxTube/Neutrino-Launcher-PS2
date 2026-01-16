EE_BIN = NeutrinoLauncher.elf
EE_OBJS = main.o
EE_LIBS = -ldebug -lpatches -lpad

all: $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
