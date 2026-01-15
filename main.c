#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <gsKit.h>
#include <dmaKit.h>
#include <libpad.h>
#include <dirent.h>
#include <string.h>
#include "ds34usb.h"

// Dati del driver incorporati (dal Makefile)
extern unsigned char ds34usb_irx[];
extern unsigned int size_ds34usb_irx;

GSGLOBAL *gsGlobal;
GSTEXTURE TexCover;
char iso_list[30][128];
int iso_count = 0, selected = 0;

void load_all_modules() {
    SifInitRpc(0);
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:PADMAN", 0, NULL);
    // Carica il driver PS3/PS4 dalla memoria interna
    SifExecModuleBuffer(ds34usb_irx, size_ds34usb_irx, 0, NULL, NULL);
    ds34usb_init();
}

int main() {
    load_all_modules();
    
    // Inizializzazione Grafica GSKit
    gsGlobal = gsKit_init_global();
    dmaKit_init_all();
    gsKit_init_screen(gsGlobal);
    
    // Scansione ISO
    DIR *d = opendir("mass:/DVD/");
    struct dirent *dir;
    while (d && (dir = readdir(d)) != NULL && iso_count < 30) {
        if (strstr(dir->d_name, ".ISO") || strstr(dir->d_name, ".iso")) {
            strncpy(iso_list[iso_count++], dir->d_name, 128);
        }
    }
    if(d) closedir(d);

    while(1) {
        gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0x00, 0x00, 0x20, 0x80, 0x00));
        
        // UI Testuale (Puoi migliorarla con i font)
        printf("DraxTube Launcher - Seleziona Gioco\n");

        // Lettura Input (Supporto DualShock 3/4 USB)
        u8 ds_data[18];
        if (ds34usb_get_status(0) & DS34USB_STATE_RUNNING) {
            ds34usb_get_data(0, ds_data);
            // Esempio: Croce su DS4 (bitmask semplificata)
            if (ds_data[5] & 0x40) break; 
        }

        gsKit_queue_exec(gsGlobal);
        gsKit_sync_flip(gsGlobal);
    }

    // Lancio Neutrino
    char full_path[256];
    sprintf(full_path, "mass:/DVD/%s", iso_list[selected]);
    char *args[] = {"mass:/neutrino.elf", "-bs=mass", "-mod=dvd", full_path, NULL};
    execv(args[0], args);

    return 0;
}
