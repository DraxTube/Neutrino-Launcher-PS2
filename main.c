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
int force_redraw = 1;

static char padBuf[256] __attribute__((aligned(64)));

void init_system() {
    SifInitRpc(0);
    init_scr();
    // Patch fondamentali per caricare ELF da USB/SD
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
    char iso_full_path[256];
    sprintf(iso_full_path, "mass:/DVD/%s", isoName);

    // Proviamo a vedere se il file esiste prima di lanciarlo
    FILE *f = fopen("mass:/neutrino.elf", "rb");
    if (f == NULL) {
        scr_clear();
        scr_setfontcolor(0x0000FF);
        scr_printf("ERRORE CRITICO:\n");
        scr_printf("Il file 'mass:/neutrino.elf' non e' leggibile.\n");
        scr_printf("1. Controlla che sia nella ROOT della USB (non in cartelle).\n");
        scr_printf("2. Rinominalo tutto in minuscolo: neutrino.elf\n");
        scr_printf("\nPremi un tasto per tornare indietro...");
        sleep(5);
        force_redraw = 1;
        return;
    }
    fclose(f);

    // Preparazione argomenti per LoadExecPS2
    struct __args {
        int argc;
        char *argv[5];
    } launch_args;

    launch_args.argc = 4;
    launch_args.argv[0] = "mass:/neutrino.elf";
    launch_args.argv[1] = "-bs=sd"; // Visto che usi MX4SIO, questo è il comando giusto
    launch_args.argv[2] = "-mod=dvd";
    launch_args.argv[3] = iso_full_path;
    launch_args.argv[4] = NULL;

    scr_clear();
    scr_printf("LANCIO IN CORSO VIA LOADEXEC...\n");
    scr_printf("Eseguo: %s\n", launch_args.argv[0]);
    
    // De-inizializza tutto prima del lancio per evitare crash
    padPortClose(0,0);
    SifExitRpc();

    // Il metodo definitivo per lanciare ELF su PS2
    LoadExecPS2(launch_args.argv[0], launch_args.argc, launch_args.argv);
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
            scr_printf("NEUTRINO LAUNCHER v1.6 - CRT OK\n");
            scr_printf("==============================\n\n");
            
            if (gameCount == 0) {
                scr_printf("Nessun gioco in mass:/DVD/ - Controlla la SD!\n");
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
            old_pad = new_pad;
        }
        for(int i=0; i<50000; i++) asm("nop");
    }
    return 0;
}
