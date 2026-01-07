#include <stdio.h>
#include <tamtypes.h>
#include <loadfile.h>
#include <kernel.h>

void launch_neutrino(char *device, char *game_id) {
    // Percorso dell'eseguibile Neutrino (deve essere sulla tua USB/HDD)
    char *elf_path = "mass:/APPS/neutrino.elf"; 
    
    // Costruiamo gli argomenti (Esempio per USB)
    // Neutrino accetta: -bs (blocksize/driver), -d (device), ecc.
    char *args[] = {
        elf_path,
        "-bs=mass",        // Cambia in 'ata' per HDD, 'sd' per MX4SIO
        "-mod=dvd", 
        game_id,           // Es: "SLES_555.01"
        NULL
    };

    printf("Lancio di Neutrino per il gioco: %s su %s\n", game_id, device);
    execv(elf_path, args);
}

int main() {
    printf("--- Neutrino GUI Launcher v0.1 ---\n");
    
    // Qui in futuro metteremo la logica per leggere i tasti del controller
    // Per ora simuliamo il lancio di un gioco su USB
    launch_neutrino("usb", "SLES_123.45");

    return 0;
}
