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
int currentDevice = 1; // Default MX4SIO
int force_redraw = 1;  // Variabile per eliminare lo sfarfallio

static char padBuf[256] __attribute__((aligned(64)));

const char* deviceNames[] = { "USB (Mass Storage)", "MX4SIO (SD Card)", "HDD (Internal)", "UDPBD (Ethernet)" };
const char* devicePrefix[] = { "mass:", "mass:", "hdd0:", "mass:" }; 
const char* neutrinoArg[]  = { "-bs=mass", "-bs=sd", "-bs=ata", "-bs=nbd" };

void init_system() {
    SifInitRpc(0);
    init_scr(); 
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
    force_redraw = 1; // Ridisegna dopo la scansione
}

void launch_neutrino(char *isoName) {
    char iso_full_path[PATH_MAX];
    sprintf(iso_full_path, "%s/DVD/%s", devicePrefix[currentDevice], isoName);

    // Lista percorsi dove Neutrino potrebbe essere nascosto
    char *paths[] = { "mass:/neutrino.elf", "mass0:/neutrino.elf", "host:/neutrino.elf", "mc0:/neutrino.elf" };

    char *args[6];
    args[1] = (char*)neutrinoArg[currentDevice]; 
    args[2] = "-mod=dvd";             
    args[3] = iso_full_path;          
    args[4] = NULL;

    scr_clear();
    scr_printf("TENTATIVO DI LANCIO...\n");
    
    for(int i=0; i<4; i++) {
        scr_printf("Provo: %s\n", paths[i]);
        args[0] = paths[i];
        execv(paths[i], args);
    }
    
    scr_setfontcolor(0x0000FF);
    scr_printf("\nERRORE: neutrino.elf non trovato!\n");
    scr_printf("Mettilo nella root della USB/SD come 'neutrino.elf'\n");
    sleep(5);
    force_redraw = 1;
}

int main() {
    init_system();
    struct padButtonStatus buttons;
    u32 old_pad = 0;
    u32 new_pad = 0;

    scan_games();

    while(1) {
        // DISEGNA SOLO SE NECESSARIO (Elimina lo sfarfallio al 100%)
        if (force_redraw) {
            scr_clear();
            scr_setfontcolor(0x00FFFF);
            scr_printf("NEUTRINO LAUNCHER v1.3 - STABILE\n");
            scr_printf("================================\n\n");
            
            scr_setfontcolor(0xFFFFFF);
            scr_printf("SORGENTE: %s\n", deviceNames[currentDevice]);
            scr_printf("GIOCHI TROVATI: %d\n\n", gameCount);

            if (gameCount == 0) {
                scr_printf("   NESSUN GIOCO IN %s/DVD/\n", devicePrefix[currentDevice]);
            } else {
                for(int i = 0; i < gameCount; i++) {
                    if (i == selectedIndex) {
                        scr_setfontcolor(0x00FF00); // Verde per il selezionato
                        scr_printf(" -> %s\n", gameList[i]);
                    } else {
                        scr_setfontcolor(0xAAAAAA); // Grigio per gli altri
                        scr_printf("    %s\n", gameList[i]);
                    }
                }
            }
            force_redraw = 0; // Reset flag
        }

        // GESTIONE INPUT
        if (padRead(0, 0, &buttons) != 0) {
            new_pad = 0xffff ^ buttons.btns;

            if ((new_pad & PAD_DOWN) && !(old_pad & PAD_DOWN)) {
                if(gameCount > 0) {
                    selectedIndex = (selectedIndex + 1) % gameCount;
                    force_redraw = 1;
                }
            }
            if ((new_pad & PAD_UP) && !(old_pad & PAD_UP)) {
                if(gameCount > 0) {
                    selectedIndex = (selectedIndex - 1 + gameCount) % gameCount;
                    force_redraw = 1;
                }
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
        
        // Risparmio CPU
        for(int i=0; i<100000; i++) asm("nop");
    }
    return 0;
}
