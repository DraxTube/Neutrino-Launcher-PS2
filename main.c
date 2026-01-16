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
int gameCount = 0;
int selectedIndex = 0;
int force_redraw = 1; // Gestisce lo sfarfallio

static char padBuf[256] __attribute__((aligned(64)));

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
    DIR *dir = opendir("mass:/DVD/");
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
    force_redraw = 1; 
}

void launch_neutrino(char *isoName) {
    char iso_path[256];
    sprintf(iso_path, "mass:/DVD/%s", isoName);

    // Argomenti per Neutrino
    char *args[] = {
        "mass:/neutrino.elf",
        "-bs=mass",   // Cambia in -bs=sd se usi SOLO MX4SIO
        "-mod=dvd",
        iso_path,
        NULL
    };

    scr_clear();
    scr_printf("Lancio in corso...\n");
    scr_printf("ISO: %s\n", iso_path);

    // Esecuzione
    execv("mass:/neutrino.elf", args);

    // Se fallisce, riprova con prefisso mass0
    execv("mass0:/neutrino.elf", args);

    scr_printf("ERRORE: neutrino.elf non trovato in mass:/\n");
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
        // SOLUZIONE SFARFALLIO: Ridisegna solo se cambia qualcosa
        if (force_redraw) {
            scr_clear();
            scr_setfontcolor(0x00FFFF);
            scr_printf("NEUTRINO LAUNCHER - FISSO\n");
            scr_printf("==========================\n\n");
            
            if (gameCount == 0) {
                scr_printf("Nessun gioco trovato in mass:/DVD/\n");
            } else {
                for(int i = 0; i < gameCount; i++) {
                    if (i == selectedIndex) {
                        scr_setfontcolor(0x00FF00); // Verde
                        scr_printf(" > %s\n", gameList[i]);
                    } else {
                        scr_setfontcolor(0xAAAAAA); // Grigio
                        scr_printf("   %s\n", gameList[i]);
                    }
                }
            }
            force_redraw = 0;
        }

        if (padRead(0, 0, &buttons) != 0) {
            new_pad = 0xffff ^ buttons.btns;

            if ((new_pad & PAD_DOWN) && !(old_pad & PAD_DOWN)) {
                if (selectedIndex < gameCount - 1) {
                    selectedIndex++;
                    force_redraw = 1;
                }
            }
            if ((new_pad & PAD_UP) && !(old_pad & PAD_UP)) {
                if (selectedIndex > 0) {
                    selectedIndex--;
                    force_redraw = 1;
                }
            }
            if ((new_pad & PAD_CROSS) && !(old_pad & PAD_CROSS)) {
                if (gameCount > 0) launch_neutrino(gameList[selectedIndex]);
            }
            old_pad = new_pad;
        }
    }
    return 0;
}
