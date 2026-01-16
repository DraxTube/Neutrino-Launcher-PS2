#include <stdio.h>
#include <string.h>      // <--- AGGIUNTO: Risolve l'errore di 'strstr'
#include <unistd.h>      // Per sleep()
#include <debug.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libpad.h>
#include <dirent.h>

// Struttura per memorizzare i dati dei giochi
typedef struct {
    char nome[128];
} Gioco;

Gioco listaGiochi[100]; // Supporto fino a 100 giochi
int totaleGiochi = 0;
int selezione = 0;

// Variabili per le Opzioni (Mode 1, 2, 3)
int mode1 = 0, mode2 = 0, mode3 = 0;

// Pulisce lo schermo (metodo semplice per l'interfaccia testuale)
void pulisci_schermo() {
    init_scr(); 
}

void mostra_menu_principale() {
    pulisci_schermo();
    
    // Header stile NHDDL
    scr_printf("\n");
    scr_printf("  DRAXTUBE LAUNCHER v1.0\n");
    scr_printf("  ----------------------\n\n");

    if (totaleGiochi == 0) {
        scr_printf("  Nessun gioco trovato in mass:/DVD/\n");
        scr_printf("  Assicurati che la USB sia inserita.\n");
    } else {
        // Mostra la lista dei giochi
        for (int i = 0; i < totaleGiochi; i++) {
            if (i == selezione) {
                // Evidenzia la selezione
                scr_printf(" > %s <\n", listaGiochi[i].nome);
            } else {
                scr_printf("   %s\n", listaGiochi[i].nome);
            }
        }

        // Footer con Info e Comandi
        scr_printf("\n  ----------------------\n");
        // Simulazione percorso copertina (come OPL)
        scr_printf("  ART: mass:/ART/%s.jpg\n", listaGiochi[selezione].nome);
        scr_printf("  ----------------------\n");
        scr_printf("  X: Avvia | /\\: Opzioni\n");
    }
}

void mostra_menu_opzioni() {
    pulisci_schermo();
    scr_printf("\n");
    scr_printf("  OPZIONI DI AVVIO\n");
    scr_printf("  ----------------\n\n");
    scr_printf("  [1] Accurate Read: %s\n", mode1 ? "ON" : "OFF");
    scr_printf("  [2] Sync Read:     %s\n", mode2 ? "ON" : "OFF");
    scr_printf("  [3] Uncached Read: %s\n", mode3 ? "ON" : "OFF");
    scr_printf("\n  ----------------\n");
    scr_printf("  O: Torna Indietro | [ ]: Cambia Mode\n");
}

void scansiona_cartella_dvd() {
    DIR *d;
    struct dirent *dir;
    
    // Prova ad aprire la cartella DVD sulla chiavetta USB
    d = opendir("mass:/DVD");
    totaleGiochi = 0;

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            // Cerca solo i file che contengono .iso o .ISO
            if (strstr(dir->d_name, ".iso") || strstr(dir->d_name, ".ISO")) {
                // Copia il nome nella lista
                snprintf(listaGiochi[totaleGiochi].nome, 128, "%s", dir->d_name);
                totaleGiochi++;
                if (totaleGiochi >= 100) break; // Limite massimo raggiunto
            }
        }
        closedir(d);
    } else {
        scr_printf("Errore: Cartella mass:/DVD non trovata.\n");
    }
}

int main() {
    // 1. Inizializzazione Sistema
    SifInitRpc(0);
    init_scr();

    scr_printf("Caricamento Driver...\n");
    
    // Caricamento moduli essenziali
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:PADMAN", 0, NULL);
    SifLoadModule("rom0:USBD", 0, NULL);
    SifLoadModule("rom0:USB_MASS", 0, NULL);

    // 2. Inizializzazione Controller
    static char padBuf[256] __attribute__((aligned(64)));
    padInit(0);
    padPortOpen(0, 0, padBuf);

    // Attesa per il montaggio della USB
    sleep(3); 

    // 3. Scansione Giochi
    scansiona_cartella_dvd();

    struct padButtonStatus buttons;
    u32 old_pad = 0;
    int in_opzioni = 0;

    // Loop Principale
    while(1) {
        int ret = padRead(0, 0, &buttons);
        
        if (ret != 0) {
            u32 paddata = 0xffff ^ buttons.btns;
            u32 new_pad = paddata & ~old_pad; // Rileva solo la nuova pressione (non tenere premuto)
            old_pad = paddata;

            if (!in_opzioni) {
                // --- MENU PRINCIPALE ---
                if ((new_pad & PAD_DOWN) && selezione < totaleGiochi - 1) { 
                    selezione++; 
                    mostra_menu_principale(); 
                }
                if ((new_pad & PAD_UP) && selezione > 0) { 
                    selezione--; 
                    mostra_menu_principale(); 
                }
                if (new_pad & PAD_TRIANGLE) { 
                    in_opzioni = 1; 
                    mostra_menu_opzioni(); 
                }
                if (new_pad & PAD_CROSS) { 
                    scr_printf("\nAvvio di %s in corso...\n", listaGiochi[selezione].nome);
                    // Qui andrà il codice per lanciare Neutrino
                    sleep(2);
                }
            } else {
                // --- MENU OPZIONI ---
                if (new_pad & PAD_CIRCLE) { 
                    in_opzioni = 0; 
                    mostra_menu_principale(); 
                }
                // Il tasto Quadrato attiva/disattiva le mode in sequenza per test
                if (new_pad & PAD_SQUARE) {
                    if (!mode1) mode1 = 1;
                    else if (!mode2) mode2 = 1;
                    else if (!mode3) mode3 = 1;
                    else { mode1=0; mode2=0; mode3=0; }
                    mostra_menu_opzioni();
                }
            }
        }
        
        // Ridisegna solo ogni tanto per evitare flickering eccessivo
        if (totaleGiochi > 0) {
             // Nota: In un'app reale si ridisegna solo se cambia qualcosa, 
             // ma qui lo lasciamo semplice.
        }
        
        usleep(50000); // 50ms di pausa per non sovraccaricare la CPU
    }

    return 0;
}
