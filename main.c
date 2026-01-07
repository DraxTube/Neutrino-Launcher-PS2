#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <stdio.h>
#include <debug.h>   // Libreria per scrivere testo semplice a schermo
#include <unistd.h>
#include <string.h>

// Funzione per lanciare Neutrino
void start_neutrino(char *mode) {
    char *neutrino_path = "mass:/neutrino.elf"; // Assicurati che sia qui!
    char *args[5];

    scr_printf("\n\nPreparazione avvio in modalita': %s\n", mode);
    
    // Argomenti per Neutrino
    args[0] = neutrino_path;
    if(strcmp(mode, "USB") == 0)      args[1] = "-bs=mass";
    else if(strcmp(mode, "HDD") == 0) args[1] = "-bs=ata";
    else if(strcmp(mode, "MX4") == 0) args[1] = "-bs=sd";
    else if(strcmp(mode, "UDP") == 0) args[1] = "-bs=nbd";
    
    args[2] = "-mod=dvd";
    // ATTENZIONE: Per questo test semplificato, lanciamo un gioco fisso o chiediamo dopo
    // Modifica questo nome con il nome ESATTO della tua ISO di prova
    args[3] = "mass:/DVD/TEST.ISO"; 
    args[4] = NULL;

    scr_printf("Esecuzione di: %s\n", neutrino_path);
    scr_printf("ISO: %s\n", args[3]);
    
    // Lancia
    execv(neutrino_path, args);
}

int main() {
    SifInitRpc(0);
    init_scr(); // Inizializza lo schermo di testo (schermo nero, scritte bianche)

    scr_printf("=================================\n");
    scr_printf("   NEUTRINO SIMPLE LAUNCHER v1   \n");
    scr_printf("=================================\n\n");

    scr_printf("Questo launcher richiede che Neutrino sia in mass:/neutrino.elf\n");
    scr_printf("E che il gioco sia mass:/DVD/TEST.ISO (per ora)\n\n");

    scr_printf("Seleziona modalita' (simulazione):\n");
    scr_printf("Attendi 5 secondi per avvio USB automatico...\n");

    // Simuliamo un'attesa (perché non abbiamo caricato i driver del pad per semplificare)
    int countdown = 5;
    while(countdown > 0) {
        scr_printf("Avvio USB tra %d...\n", countdown);
        sleep(1); // Aspetta 1 secondo
        countdown--;
    }

    start_neutrino("USB");

    // Se fallisce:
    scr_printf("\nERRORE: Neutrino non trovato o errore di esecuzione.\n");
    while(1) {} // Blocca qui

    return 0;
}
