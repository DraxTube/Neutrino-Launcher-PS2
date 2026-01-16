#include <stdio.h>
#include <debug.h>
#include <loadfile.h>
#include <sifrpc.h>
#include "ds34usb_client.h"

// Dati del driver incorporato (gestiti dal Makefile)
extern unsigned char ds34usb_irx[];
extern unsigned int size_ds34usb_irx;

int main() {
    SifInitRpc(0);
    init_scr();
    
    // --- CREDITI DRAXTUBE ---
    scr_printf("==========================================\n");
    scr_printf("      NEUTRINO LAUNCHER - DraxTube        \n");
    scr_printf("   GitHub: https://github.com/DraxTube    \n");
    scr_printf("   YouTube: @DraxTube01                   \n");
    scr_printf("==========================================\n\n");
    
    scr_printf("Caricamento driver USB in corso...\n");
    
    // Carichiamo i moduli necessari della PS2
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:USBD", 0, NULL);
    
    // Carichiamo il driver compilato nella memoria
    SifExecModuleBuffer(ds34usb_irx, size_ds34usb_irx, 0, NULL, NULL);
    
    if(ds34usb_init()) {
        scr_printf("Stato: Driver PS3/PS4 ATTIVO!\n");
    }

    scr_printf("\nPremi il tasto CROCE per avviare il gioco...\n");

    u8 data[18];
    while(1) {
        if (ds34usb_get_status(0) & 0x08) { // DS34USB_STATE_RUNNING
            ds34usb_get_data(0, data);
            if (data[5] & 0x40) { // Tasto Croce
                scr_printf("Comando ricevuto! Lancio Neutrino...\n");
                break;
            }
        }
    }

    return 0;
}
