#include <stdio.h>
#include <debug.h>
#include <loadfile.h>
#include <sifrpc.h>
#include "ds34usb_client.h"

// Simboli del driver incorporato creati dal Makefile
extern unsigned char ds34usb_irx[];
extern unsigned int size_ds34usb_irx;

int main() {
    SifInitRpc(0);
    init_scr();
    
    scr_printf("DraxTube Neutrino Launcher - Progetto GitHub\n");
    
    // Carichiamo i moduli necessari
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:USBD", 0, NULL);
    
    // Carichiamo il driver che abbiamo compilato dentro l'ELF
    SifExecModuleBuffer(ds34usb_irx, size_ds34usb_irx, 0, NULL, NULL);
    
    ds34usb_init();
    scr_printf("Controller PS3/PS4 USB Pronto!\n");

    u8 data[18];
    while(1) {
        if (ds34usb_get_status(0) & DS34USB_STATE_RUNNING) {
            ds34usb_get_data(0, data);
            if (data[5] & 0x40) { // Tasto Croce
                scr_printf("Croce premuta! Lancio Neutrino...\n");
                break;
            }
        }
    }
    return 0;
}
