#include <stdio.h>
#include <debug.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libpad.h>

// Buffer per il controller standard PS2
static char padBuf[256] __attribute__((aligned(64)));

int main() {
    SifInitRpc(0);
    init_scr();

    // --- CREDITI DraxTube ---
    scr_printf("==========================================\n");
    scr_printf("      NEUTRINO LAUNCHER - DraxTube        \n");
    scr_printf("   YouTube: @DraxTube01                   \n");
    scr_printf("   GitHub: https://github.com/DraxTube    \n");
    scr_printf("==========================================\n\n");

    scr_printf("Caricamento moduli di sistema...\n");
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:PADMAN", 0, NULL);

    padInit(0);
    padPortOpen(0, 0, padBuf);

    scr_printf("Sistema pronto. Usa un controller PS2 standard.\n");
    scr_printf("Premi START per avviare Neutrino...\n");

    struct padButtonStatus buttons;
    while(1) {
        int state = padGetState(0, 0);
        if (state == PAD_STATE_STABLE || state == PAD_STATE_FINDCTP1) {
            int ret = padRead(0, 0, &buttons);
            if (ret != 0) {
                u32 paddata = 0xffff ^ buttons.btns;
                if (paddata & PAD_START) {
                    scr_printf("START premuto! Avvio in corso...\n");
                    break;
                }
            }
        }
    }

    // Qui andrà il comando per lanciare Neutrino
    return 0;
}
