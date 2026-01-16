#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <stdio.h>
#include <gsKit.h>
#include <dmaKit.h>
#include <gsToolkit.h>
#include <libpad.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include "ds34usb.h"

// Simboli per il driver incorporato
extern unsigned char ds34usb_irx[];
extern unsigned int size_ds34usb_irx;

GSGLOBAL *gsGlobal;
GSTEXTURE TexCover;
char iso_list[30][128];
int iso_count = 0, selected = 0;
static char padBuf[256] __attribute__((aligned(64)));

void load_modules() {
    SifInitRpc(0);
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:PADMAN", 0, NULL);
    // Carica il driver PS3/PS4 dalla memoria dell'ELF
    SifExecModuleBuffer(ds34usb_irx, size_ds34usb_irx, 0, NULL, NULL);
    ds34usb_init();
}

int main() {
    load_modules();
    
    gsGlobal = gsKit_init_global();
    dmaKit_init_all();
    gsKit_init_screen(gsGlobal);
    
    DIR *d = opendir("mass:/DVD/");
    struct dirent *dir;
    while (d && (dir = readdir(d)) != NULL && iso_count < 30) {
        if (strstr(dir->d_name, ".ISO") || strstr(dir->d_name, ".iso")) {
            strncpy(iso_list[iso_count++], dir->d_name, 128);
        }
    }
    if(d) closedir(d);

    while(1) {
        gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0x00, 0x00, 0x25, 0x80, 0x00));
        
        // Logica di rendering (semplificata per stabilità)
        if (ds34usb_get_status(0) & DS34USB_STATE_RUNNING) {
            u8 data[18];
            ds34usb_get_data(0, data);
            if (data[5] & 0x40) break; // Esempio: tasto Croce per avviare
        }

        gsKit_queue_exec(gsGlobal);
        gsKit_sync_flip(gsGlobal);
    }

    char full_path[256];
    sprintf(full_path, "mass:/DVD/%s", iso_list[selected]);
    char *args[] = {"mass:/neutrino.elf", "-bs=mass", "-mod=dvd", full_path, NULL};
    execv(args[0], args);
    return 0;
}
