EE_BIN = NeutrinoLauncher.elf
EE_OBJS = main.o ds34usb_client.o iop/ds34usb.irx.o
EE_LIBS = -ldebug -lpatches -lfileXio

# Regola speciale per includere il driver senza usare bin2s
iop/ds34usb.irx.o: iop/ds34usb.irx
	cd iop && $(IOP_OBJCOPY) -I binary -O elf32-iop -B mips ds34usb.irx ds34usb.irx.o
	cp iop/ds34usb.irx.o .
	$(EE_OBJCOPY) -I elf32-iop -O elf32-littlemips iop/ds34usb.irx.o ds34usb.irx.o
	# Creiamo i simboli per il codice C
	$(EE_NM) -g ds34usb.irx.o | grep " _binary_ds34usb_irx_start" | awk '{print "unsigned char ds34usb_irx[] __attribute__((aligned(16))) = {0};"}' > tmp.c
	# Nota: per semplicità nel main.c useremo i nomi generati da objcopy

iop/ds34usb.irx:
	$(MAKE) -C iop/

all: $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS) ds34usb.irx.o
	$(MAKE) -C iop/ clean

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
