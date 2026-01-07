#include <stdio.h>
#include <tamtypes.h>
#include <loadfile.h>
#include <kernel.h>
#include <fileXio_rpc.h>

// Funzione per caricare i moduli necessari (IRX)
void load_basic_modules() {
    SifInitRpc(0);
    fxeInit(); // Inizializza fileXio per accesso a HDD/USB
}

void launch_game(const char* path, const char* mode) {
    char *elf_path = "mass:/neutrino.elf"; // Assumi che Neutrino sia qui
    
    // Costruzione dinamica degli argomenti in base al dispositivo
    char *args[10];
    args[0] = elf_path;
    
    if (strcmp(mode, "usb") == 0) {
        args[1] = "-bs=mass";
    } else if (strcmp(mode, "mx4sio") == 0) {
        args[1] = "-bs=sd";
    } else if (strcmp(mode, "hdd") == 0) {
        args[1] = "-bs=ata";
    }

    args[2] = "-mod=dvd";
    args[3] = (char*)path; // Percorso della ISO
    args[4] = NULL;

    printf("Avvio in corso su %s...\n", mode);
    execv(elf_path, args);
}

int main() {
    load_basic_modules();
    
    printf("Neutrino Multi-Launcher Ready.\n");
    printf("1. USB (mass)\n2. MX4SIO (sd)\n3. HDD (ata)\n");

    // Per ora lanciamo un test predefinito
    // In futuro qui leggeremo l'input del controller PS2
    launch_game("mass:/DVD/GIOCO.ISO", "usb");

    return 0;
}
