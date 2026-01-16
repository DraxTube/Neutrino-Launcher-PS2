EE_BIN = NeutrinoLauncher.elf
EE_OBJS = main.o ds34usb.o ds34usb_driver_bin.o
EE_LIBS = -lgskit -ldmakit -ljpeg -lpng -lz -lpad -ldebug -lpatches -lfileXio

all: $(EE_BIN)

# Prende il file ds34usb.irx e lo trasforma in codice C
ds34usb_driver_bin.o: ds34usb.irx
	bin2s ds34usb.irx ds34usb_driver_bin.s ds34usb_irx
	mips64r5900el-ps2-elf-gcc -c ds34usb_driver_bin.s -o ds34usb_driver_bin.o

clean:
	rm -f $(EE_BIN) $(EE_OBJS) ds34usb_driver_bin.s

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
