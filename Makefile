EE_BIN = NeutrinoLauncher.elf
EE_OBJS = main.o ds34usb_client.o ds34usb_irx.o
EE_LIBS = -ldebug -lpatches -lfileXio

# Trasforma il driver compilato in un oggetto per il launcher
ds34usb_irx.o: iop/ds34usb.irx
	bin2s iop/ds34usb.irx ds34usb_irx.s ds34usb_irx
	$(EE_AS) ds34usb_irx.s -o ds34usb_irx.o

# Compila la cartella iop prima del resto
iop/ds34usb.irx:
	$(MAKE) -C iop/

all: iop/ds34usb.irx.o $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS) ds34usb_irx.s iop/ds34usb.irx
	$(MAKE) -C iop/ clean

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
