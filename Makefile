EE_BIN = NeutrinoLauncher.elf
EE_OBJS = main.o ds34usb_client.o iop/ds34usb.irx.o
EE_LIBS = -ldebug -lpatches -lfileXio

# Regola per trasformare il driver IRX in un oggetto caricabile nel launcher
iop/ds34usb.irx.o: iop/ds34usb.irx
	bin2s iop/ds34usb.irx iop/ds34usb.irx.s ds34usb_irx
	$(EE_AS) iop/ds34usb.irx.s -o iop/ds34usb.irx.o

# Compilazione del driver IOP (richiede Makefile specifico in iop/)
iop/ds34usb.irx:
	$(MAKE) -C iop/

all: iop/ds34usb.irx.o $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS) iop/ds34usb.irx.o
	$(MAKE) -C iop/ clean

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
