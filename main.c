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

// DEFINIZIONI FISSE PER IL TUO SETUP
#define NEUTRINO_PATH "mass:/neutrino.elf"  // Neutrino è sulla USB
#define GAMES_PREFIX  "mass1:/DVD/"         // I giochi sono su MX4SIO (di solito mass1)

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
    // Proviamo mass1: (MX4SIO) e se fallisce mass0:
    DIR *dir = opendir(GAMES_PREFIX);
    if (dir == NULL) {
        dir = opendir("mass0:/DVD/");
    }

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
    
    // Costruiamo il percorso del gioco (su MX4SIO)
    // Usiamo mass1: perché Neutrino riconoscerà MX4SIO così o tramite il driver interno
    sprintf(iso_full_path, "mass1:/DVD/%s", isoName);

    // Argomenti per Neutrino
    char *args[6];
    args[0] = NEUTRINO_PATH;
    args[1] = "-bs=sd";      // IMPORTANTE: Dice a Neutrino di usare MX4SIO
    args[2] = "-mod=dvd";
    args[3] = iso_full_path;
    args[4] = NULL;

    scr_clear();
    scr_printf("LANCIO IN CORSO...\n");
    scr_printf("ELF: %s (da USB)\n", NEUTRINO_PATH);
    scr_printf("ISO: %s (da MX4SIO)\n", iso_full_path);

    // Controllo se Neutrino esiste su USB prima di spegnere tutto
    FILE *f = fopen(NEUTRINO_PATH, "rb");
    if (f) {
        fclose(f);
        SifExitRpc();
        // LoadExecPS2 carica l'ELF da USB e gli passa il percorso del gioco su SD
        LoadExecPS2(NEUTRINO_PATH, 4, args);
    } else {
        scr_setfontcolor(0x0000FF);
        scr_printf("\nERRORE: %s NON TROVATO SU USB!\n", NEUTRINO_PATH);
        scr_printf("Metti neutrino.elf nella chiavetta USB.\n");
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
            scr_printf("NEUTRINO LAUNCHER v1.8 - USB+MX4SIO\n");
            scr_printf("======================================\n\n");
            
            if (gameCount == 0) {
                scr_printf("GIOCHI NON TROVATI SU MX4SIO (mass1:/DVD/)\n");
                scr_printf("Premi SELECT per riprovare.\n");
            } else {
                scr_setfontcolor(0xFFFFFF);
                scr_printf("Seleziona Gioco su SD:\n");
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
