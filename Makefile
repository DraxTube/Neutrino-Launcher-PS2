EE_BIN = NeutrinoLauncher.elf
# Includiamo tutti i componenti
EE_OBJS = main.o ds34usb.o ds34usb_driver_bin.o
EE_LIBS = -lgskit -ldmakit -ljpeg -lpng -lz -lpad -ldebug -lpatches -lfileXio

all: $(EE_BIN)

# Trasforma il driver in codice
ds34usb_driver_bin.o: ds34usb.irx
	bin2s ds34usb.irx ds34usb_driver_bin.s ds34usb_irx
	mips64r5900el-ps2-elf-gcc -c ds34usb_driver_bin.s -o ds34usb_driver_bin.o

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
