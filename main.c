#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <sbv_patches.h>

// Inizializza la console e applica le patch necessarie
void init_system() {
    // Inizializza RPC
    SifInitRpc(0);

    // Applica le patch SBV per permettere il caricamento di moduli ed ELF
    // Questo è FONDAMENTALE per i launcher moderni
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();
}

void launch_game(const char* path, const char* mode) {
    // Percorso assoluto dell'eseguibile di Neutrino
    // NOTA: Assicurati che il percorso sia corretto sulla tua USB
    char *elf_path = "mass:/neutrino.elf"; 
    
    char *args[5];
    
    // Argomento 0: Il percorso dell'eseguibile stesso
    args[0] = elf_path;
    
    // Argomento 1: Il driver (Block Device)
    if (strcmp(mode, "usb") == 0) {
        args[1] = "-bs=mass";
    } else if (strcmp(mode, "mx4sio") == 0) {
        args[1] = "-bs=sd";
    } else if (strcmp(mode, "hdd") == 0) {
        args[1] = "-bs=ata";
    } else if (strcmp(mode, "udpbd") == 0) {
        args[1] = "-bs=nbd";
    } else {
        // Default a mass se non specificato
        args[1] = "-bs=mass";
    }

    // Argomento 2: Modalità
    args[2] = "-mod=dvd";
    
    // Argomento 3: Percorso ISO
    args[3] = (char*)path;
    
    // Argomento 4: Terminatore
    args[4] = NULL;

    printf("Tentativo di avvio:\nELF: %s\nMode: %s\nISO: %s\n", elf_path, args[1], args[3]);
    
    // execv sostituisce il processo corrente con quello nuovo
    execv(elf_path, args);
}

int main() {
    init_system();
    
    printf("Neutrino Multi-Launcher v0.1\n");
    printf("----------------------------\n");

    // Simulazione avvio. In futuro qui metteremo il menu grafico.
    // Prova ad avviare un gioco USB fittizio
    launch_game("mass:/DVD/GIOCO.ISO", "usb");

    // Se execv fallisce, il codice continua qui
    printf("Errore: Impossibile lanciare Neutrino. Controlla il percorso.\n");
    
    // Loop infinito per non far spegnere la console subito
    while(1) {}

    return 0;
}
