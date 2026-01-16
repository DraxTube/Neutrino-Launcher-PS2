#include <stdio.h>
#include <debug.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libpad.h>
#include <dirent.h>      // Standard POSIX per le cartelle
#include <unistd.h>      // Per sleep()

// Struttura per gestire i giochi
typedef struct {
    char nome[128];
    char percorso[256];
} Gioco;

Gioco listaGiochi[50];
int totaleGiochi = 0;
int selezione = 0;

// Variabili per le Opzioni (tipo Mode di OPL)
int mode1 = 0, mode2 = 0, mode3 = 0;

void pulisci_schermo() {
    init_scr(); // Resetta lo schermo del debugger
}

void mostra_interfaccia() {
    pulisci_schermo();
    scr_printf("==========================================\n");
    scr_printf("      DRAXTUBE LAUNCHER - PROGETTO        \n");
    scr_printf("==========================================\n\n");

    if (totaleGiochi == 0) {
        scr_printf("Nessun gioco trovato in mass:/DVD/\n");
    } else {
        for (int i = 0; i < totaleGiochi; i++) {
            if (i == selezione)
                scr_printf("> [%s] <\n", listaGiochi[i].nome);
            else
                scr_printf("  %s  \n", listaGiochi[i].nome);
        }
        
        scr_printf("\n------------------------------------------\n");
        scr_printf("INFO: mass:/ART/%s.jpg (Copertina)\n", listaGiochi[selezione].nome);
        scr_printf("------------------------------------------\n");
        scr_printf("X: Avvia | TRIANGOLO: Opzioni | START: Esci\n");
    }
}

void menu_opzioni() {
    pulisci_schermo();
    scr_printf("=== OPZIONI COMPATIBILITA' (MODES) ===\n\n");
    scr_printf(" [1] Mode 1 (Accurate Read): %s\n", mode1 ? "ON" : "OFF");
    scr_printf(" [2] Mode 2 (Sync Read):     %s\n", mode2 ? "ON" : "OFF");
    scr_printf(" [3] Mode 3 (Uncached):      %s\n", mode3 ? "ON" : "OFF");
    scr_printf("\nPremi CERCHIO per tornare indietro...\n");
}

void scansiona_giochi() {
    DIR *d;
    struct dirent *dir;
    d = opendir("mass:/DVD");
    totaleGiochi = 0;

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strstr(dir->d_name, ".iso") || strstr(dir->d_name, ".ISO")) {
                snprintf(listaGiochi[totaleGiochi].nome, 128, "%s", dir->d_name);
                totaleGiochi++;
            }
        }
        closedir(d);
    }
}

int main() {
    SifInitRpc(0);
    init_scr();

    // Caricamento silenzioso dei driver
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:PADMAN", 0, NULL);
    SifLoadModule("rom0:USBD", 0, NULL);
    SifLoadModule("rom0:USB_MASS", 0, NULL);

    static char padBuf[256] __attribute__((aligned(64)));
    padInit(0);
    padPortOpen(0, 0, padBuf);

    scr_printf("Inizializzazione DraxTube...\n");
    sleep(2); // Sostituito delay() con sleep() per evitare errori

    scansiona_giochi();

    struct padButtonStatus buttons;
    int in_opzioni = 0;

    while(1) {
        if (padRead(0, 0, &buttons) != 0) {
            u32 paddata = 0xffff ^ buttons.btns;

            if (!in_opzioni) {
                if (paddata & PAD_DOWN && selezione < totaleGiochi - 1) { selezione++; mostra_interfaccia(); }
                if (paddata & PAD_UP && selezione > 0) { selezione--; mostra_interfaccia(); }
                if (paddata & PAD_TRIANGLE) { in_opzioni = 1; menu_opzioni(); }
                if (paddata & PAD_CROSS) { 
                    scr_printf("\nAvvio di %s con Neutrino...\n", listaGiochi[selezione].nome);
                    // Qui chiameremo il boot di Neutrino
                }
            } else {
                if (paddata & PAD_CIRCLE) { in_opzioni = 0; mostra_interfaccia(); }
                // Cambia le mode
                if (paddata & PAD_SQUARE) { mode1 = !mode1; menu_opzioni(); }
            }
        }
        mostra_interfaccia();
        usleep(150000); // Piccola pausa per non far scorrere la lista troppo veloce
    }

    return 0;
}
