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

#define MAX_GAMES 100
char gameList[MAX_GAMES][64];
char activeDevice[10]; // Memorizza se è mass:, mass0:, mass1:, ecc.
int gameCount = 0;
int selectedIndex = 0;
int force_redraw = 1;

static char padBuf[256] __attribute__((aligned(64)));

void init_system() {
    SifInitRpc(0);
    init_scr();
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();
    padInit(0);
    padPortOpen(0, 0, padBuf);
}

// Prova a scansionare mass:, mass0:, mass1: finché non trova la cartella DVD
void scan_games() {
    gameCount = 0;
    char *prefixes[] = {"mass:", "mass0:", "mass1:", "mass2:"};
    
    for (int i = 0; i < 4; i++) {
        char path[32];
        sprintf(path, "%s/DVD/", prefixes[i]);
        DIR *dir = opendir(path);
        if (dir != NULL) {
            strcpy(activeDevice, prefixes[i]); // Abbiamo trovato il dispositivo giusto!
            struct dirent *ent;
            while ((ent = readdir(dir)) != NULL && gameCount < MAX_GAMES) {
                if (strstr(ent->d_name, ".ISO") || strstr(ent->d_name, ".iso")) {
                    strncpy(gameList[gameCount], ent->d_name, 63);
                    gameCount++;
                }
            }
            closedir(dir);
            if (gameCount > 0) break; // Esci se hai trovato giochi
        }
    }
    force_redraw = 1;
}

void launch_neutrino(char *isoName) {
    char neutrino_path[64];
    char iso_full_path[256];
    
    sprintf(neutrino_path, "%s/neutrino.elf", activeDevice);
    sprintf(iso_full_path, "%s/DVD/%s", activeDevice, isoName);

    // Argomenti per Neutrino (Pre-Alpha potrebbe richiedere -bs=sd per MX4SIO)
    char *args[5];
    args[0] = neutrino_path;
    args[1] = "-bs=sd"; // Cambialo in "-bs=mass" se usi una chiavetta USB normale
    args[2] = "-mod=dvd";
    args[3] = iso_full_path;
    args[4] = NULL;

    scr_clear();
    scr_printf("LANCIO DA: %s\n", activeDevice);
    scr_printf("ELF: %s\n", neutrino_path);

    // Controllo esistenza file prima del boot
    FILE *f = fopen(neutrino_path, "rb");
    if (f) {
        fclose(f);
        SifExitRpc();
        LoadExecPS2(neutrino_path, 4, args);
    } else {
        scr_setfontcolor(0x0000FF);
        scr_printf("ERRORE: %s non trovato!\n", neutrino_path);
        scr_printf("Rinomina il file in neutrino.elf (tutto minuscolo).\n");
        sleep(5);
        force_redraw = 1;
    }
}

int main() {
    init_system();
    struct padButtonStatus buttons;
    u32 old_pad = 0;
    u32 new_pad = 0;

    scan_games();

    while(1) {
        if (force_redraw) {
            scr_clear();
            scr_setfontcolor(0x00FFFF);
            scr_printf("NEUTRINO LAUNCHER v1.7 - PORT: %s\n", activeDevice);
            scr_printf("======================================\n\n");
            
            if (gameCount == 0) {
                scr_printf("ERRORE: Nessun gioco trovato in mass0,1,2:/DVD/\n");
                scr_printf("Premi SELECT per riprovare la scansione.\n");
            } else {
                scr_setfontcolor(0xFFFFFF);
                for(int i = 0; i < gameCount; i++) {
                    if (i == selectedIndex) {
                        scr_setfontcolor(0x00FF00);
                        scr_printf(" > %s\n", gameList[i]);
                    } else {
                        scr_setfontcolor(0xAAAAAA);
                        scr_printf("   %s\n", gameList[i]);
                    }
                }
            }
            force_redraw = 0;
        }

        if (padRead(0, 0, &buttons) != 0) {
            new_pad = 0xffff ^ buttons.btns;
            if ((new_pad & PAD_DOWN) && !(old_pad & PAD_DOWN)) {
                if (selectedIndex < gameCount - 1) { selectedIndex++; force_redraw = 1; }
            }
            if ((new_pad & PAD_UP) && !(old_pad & PAD_UP)) {
                if (selectedIndex > 0) { selectedIndex--; force_redraw = 1; }
            }
            if ((new_pad & PAD_CROSS) && !(old_pad & PAD_CROSS)) {
                if (gameCount > 0) launch_neutrino(gameList[selectedIndex]);
            }
            if ((new_pad & PAD_SELECT) && !(old_pad & PAD_SELECT)) {
                scan_games();
            }
            old_pad = new_pad;
        }
        for(int i=0; i<50000; i++) asm("nop");
    }
    return 0;
}
