#include <stdio.h>
#include <debug.h>
#include <loadfile.h>
#include <sifrpc.h>
#include "ds34usb_client.h"

// Simboli creati automaticamente dal sistema
extern unsigned char _binary_ds34usb_irx_start[];
extern unsigned char _binary_ds34usb_irx_size[];

int main() {
    SifInitRpc(0);
    init_scr();
    
    // --- CREDITI DraxTube ---
    scr_printf("==========================================\n");
    scr_printf("      NEUTRINO LAUNCHER - DraxTube        \n");
    scr_printf("   YouTube: @DraxTube01                   \n");
    scr_printf("   GitHub: https://github.com/DraxTube    \n");
    scr_printf("==========================================\n\n");
    
    scr_printf("Inizializzazione USB...\n");
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:USBD", 0, NULL);
    
    // Carichiamo il driver integrato
    unsigned int size = (unsigned int)_binary_ds34usb_irx_size;
    SifExecModuleBuffer(_binary_ds34usb_irx_start, size, 0, NULL, NULL);
    
    if(ds34usb_init()) {
        scr_printf("Stato: Controller PS3/PS4 PRONTO!\n");
    }

    scr_printf("\nPremi CROCE per avviare il gioco...\n");

    u8 data[18];
    while(1) {
        if (ds34usb_get_status(0) & 0x08) {
            ds34usb_get_data(0, data);
            if (data[5] & 0x40) { // Tasto Croce
                scr_printf("Avvio in corso...\n");
                break;
            }
        }
    }
    return 0;
}
