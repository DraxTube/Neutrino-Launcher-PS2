EE_BIN = NeutrinoLauncher.elf
EE_OBJS = main.o ds34usb_client.o ds34usb.irx.o
EE_LIBS = -ldebug -lpatches -lfileXio

# Regola per includere il driver dentro il launcher
ds34usb.irx.o: iop/ds34usb.irx
	bin2s iop/ds34usb.irx ds34usb.irx.s ds34usb_irx
	$(EE_AS) ds34usb.irx.s -o ds34usb.irx.o

# Compila prima il driver nella cartella iop/
iop/ds34usb.irx:
	$(MAKE) -C iop/

all: $(EE_BIN)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
