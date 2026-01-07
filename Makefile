EE_BIN = NeutrinoLauncher.elf
EE_OBJS = main.o
# Rimuoviamo -lfileXio. Aggiungiamo -lpatches per gestire i driver.
EE_LIBS = -ldebug -lpatches

all: $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
