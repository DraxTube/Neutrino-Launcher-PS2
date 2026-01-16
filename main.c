#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <debug.h>
#include <libpad.h>
#include <sbv_patches.h>
#include <limits.h>

#define MAX_GAMES 100

char gameList[MAX_GAMES][64];
int gameCount = 0;
int selectedIndex = 0;
int currentDevice = 1; // Default su 1 (MX4SIO) visto che usi quello

static char padBuf[256] __attribute__((aligned(64)));

const char* deviceNames[] = { "USB (Mass Storage)", "MX4SIO (SD Card)", "HDD (Internal)", "UDPBD (Ethernet)" };
// Per MX4SIO e USB, il prefisso di lettura file in molti driver è "mass:" 
// ma per Neutrino il driver del blocco cambia.
const char* devicePrefix[] = { "mass:", "mass:", "hdd0:", "mass:" }; 
const char* neutrinoArg[]  = { "-bs=mass", "-bs=sd", "-bs=ata", "-bs=nbd" };

void init_system() {
    SifInitRpc(0);
    init_scr(); 
    // Impostiamo un colore di sfondo fisso per ridurre il fastidio del refresh
    scr_setfontcolor(0xFFFFFF);
    
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();

    padInit(0);
    padPortOpen(0, 0, padBuf);
}

void scan_games() {
    gameCount = 0;
    selectedIndex = 0;
    char path[PATH_MAX];
    sprintf(path, "%s/DVD/", devicePrefix[currentDevice]);
    
    DIR *dir = opendir(path);
    if (dir != NULL) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL && gameCount < MAX_GAMES) {
            if (strstr(ent->d_name, ".ISO") || strstr(ent->d_name, ".iso")) {
                strncpy(gameList[gameCount], ent->d_name, 63);
                gameCount++;
            }
        }
        closedir(dir);
    }
}

void launch_neutrino(char *isoName) {
    // Proviamo a cercare neutrino.elf sia nella root che in una cartella APPS
    char elf_path[] = "mass:/neutrino.elf"; 
    char iso_full_path[PATH_MAX];
    sprintf(iso_full_path, "%s/DVD/%s", devicePrefix[currentDevice], isoName);

    char *args[6];
    args[0] = elf_path;               
    args[1] = (char*)neutrinoArg[currentDevice]; 
    args[2] = "-mod=dvd";             
    args[3] = iso_full_path;          
    args[4] = NULL;

    scr_clear();
    scr_printf("AVVIO IN CORSO...\n------------------\n");
    scr_printf("Driver: %s\n", args[1]);
    scr_printf("Gioco: %s\n", isoName);
    
    // Piccolo delay per permettere ai driver di stabilizzarsi
    sleep(1);
    
    execv(elf_path, args);
    
    // Se execv fallisce, proviamo un percorso alternativo (es. se lanciato da uLaunchELF)
    execv("host:/neutrino.elf", args); 
    
    scr_printf("\nERRORE: neutrino.elf non trovato!\n");
    scr_printf("Assicurati che sia nella root della USB o SD\ne che si chiami esattamente neutrino.elf\n");
    sleep(5);
}

int main() {
    init_system();
    struct padButtonStatus buttons;
    u32 old_pad = 0;
    u32 new_pad = 0;

    scan_games();

    while(1) {
        // --- IL TRUCCO PER IL TUBO CATODICO (V-Sync) ---
        // Aspettiamo che la TV abbia finito di disegnare il frame
        scr_vblankWait(); 
        
        scr_clear();
        scr_setfontcolor(0xFFFF00); // Giallo per il titolo
        scr_printf("NEUTRINO MULTI-LAUNCHER v1.1\n");
        scr_printf("============================\n\n");
        
        scr_setfontcolor(0xFFFFFF);
        scr_printf("DISPOSITIVO: %s\n", deviceNames[currentDevice]);
        scr_printf("(L1/R1 cambia - X seleziona)\n\n");

        if (gameCount == 0) {
            scr_printf("   NESSUN GIOCO IN %s/DVD/\n", devicePrefix[currentDevice]);
        } else {
            for(int i=0; i<gameCount; i++) {
                if (i == selectedIndex) {
                    scr_setfontcolor(0x00FF00); // Verde selezione
                    scr_printf(" -> %s\n", gameList[i]);
                } else {
                    scr_setfontcolor(0xAAAAAA); // Grigio
                    scr_printf("    %s\n", gameList[i]);
                }
            }
        }

        if (padRead(0, 0, &buttons) != 0) {
            new_pad = 0xffff ^ buttons.btns;

            if ((new_pad & PAD_DOWN) && !(old_pad & PAD_DOWN)) {
                if(gameCount > 0) selectedIndex = (selectedIndex + 1) % gameCount;
            }
            if ((new_pad & PAD_UP) && !(old_pad & PAD_UP)) {
                if(gameCount > 0) selectedIndex = (selectedIndex - 1 + gameCount) % gameCount;
            }
            if ((new_pad & PAD_R1) && !(old_pad & PAD_R1)) {
                currentDevice = (currentDevice + 1) % 4;
                scan_games();
            }
            if ((new_pad & PAD_L1) && !(old_pad & PAD_L1)) {
                currentDevice = (currentDevice - 1 + 4) % 4;
                scan_games();
            }
            if ((new_pad & PAD_CROSS) && !(old_pad & PAD_CROSS)) {
                if (gameCount > 0) launch_neutrino(gameList[selectedIndex]);
            }
            old_pad = new_pad;
        }
    }
    return 0;
}
