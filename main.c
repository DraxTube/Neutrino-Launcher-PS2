#include <stdio.h>
#include <debug.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <fileXio_rpc.h> // Per leggere i file dalla USB
#include <libpad.h>
#include <dirent.h>      // Per gestire le cartelle

void scansiona_giochi() {
    scr_printf("Scansione cartella mass:/DVD/...\n");
    
    int fd = fileXioDopen("mass:/DVD");
    if (fd < 0) {
        scr_printf("ERRORE: Cartella DVD non trovata su USB!\n");
        return;
    }

    iox_dirent_t record;
    int count = 0;
    while (fileXioDread(fd, &record) > 0) {
        // Mostriamo solo i file (non le cartelle)
        if (record.stat.mode & FIO_S_IFREG) {
            scr_printf(" [%d] Gioco trovato: %s\n", count + 1, record.name);
            count++;
        }
    }
    fileXioDclose(fd);

    if (count == 0) scr_printf("Nessun file ISO trovato.\n");
}

int main() {
    SifInitRpc(0);
    init_scr();

    // --- CREDITI DraxTube ---
    scr_printf("==========================================\n");
    scr_printf("      DRAXTUBE LAUNCHER (NHDDL Style)     \n");
    scr_printf("   YouTube: @DraxTube01                   \n");
    scr_printf("==========================================\n\n");

    // Caricamento driver necessari
    scr_printf("Avvio USB e FileSystem...\n");
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:USBD", 0, NULL);     // Driver USB
    SifLoadModule("rom0:USB_MASS", 0, NULL); // Driver Mass Storage
    SifLoadModule("rom0:FILEXIO", 0, NULL);  // Driver per gestione file
    
    fileXioInit(); // Inizializza la libreria fileXio

    // Aspettiamo un secondo che la USB venga letta
    scr_printf("In attesa della chiavetta USB...\n");
    delay(2); 

    // Eseguiamo la scansione proprio come NHDDL
    scansiona_giochi();

    scr_printf("\nPremi START per ricaricare la lista...\n");

    // Loop infinito per ora
    while(1) {
        // Qui aggiungeremo la selezione del gioco con le frecce
    }

    return 0;
}
