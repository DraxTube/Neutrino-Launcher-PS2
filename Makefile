EE_BIN = NeutrinoLauncher.elf
EE_OBJS = main.o
# Aggiungi -lfileXio qui sotto
EE_LIBS = -ldebug -lpad -lfileXio -lpatches

all: $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
