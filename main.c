#include <stdio.h>
#include <debug.h>
#include <loadfile.h>
#include <sifrpc.h>
#include "ds34usb_client.h"

// Simboli del driver incorporato
extern unsigned char ds34usb_irx[];
extern unsigned int size_ds34usb_irx;

int main() {
    SifInitRpc(0);
    init_scr();
    
    scr_printf("======================================\n");
    scr_printf("   DRAXTUBE NEUTRINO LAUNCHER PRO     \n");
    scr_printf("   YouTube: @DraxTube01               \n");
    scr_printf("======================================\n");
    
    // Carichiamo i moduli base della PS2
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:USBD", 0, NULL);
    
    // Carichiamo il TUO driver (quello che GitHub compilerà)
    SifExecModuleBuffer(ds34usb_irx, size_ds34usb_irx, 0, NULL, NULL);
    
    if(ds34usb_init()) {
        scr_printf("Driver DualShock 3/4 attivato!\n");
    }

    u8 data[18];
    scr_printf("\nCollega il controller e premi CROCE...\n");

    while(1) {
        if (ds34usb_get_status(0) & DS34USB_STATE_RUNNING) {
            ds34usb_get_data(0, data);
            if (data[5] & 0x40) { // Tasto Croce
                scr_printf("Tasto CROCE rilevato! Avvio Neutrino...\n");
                break;
            }
        }
    }

    return 0;
}
