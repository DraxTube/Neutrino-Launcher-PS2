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

// Configurazione basata sul tuo setup e README
#define NEUTRINO_PATH "mass:/neutrino.elf"
#define GAMES_PREFIX  "mass1:/DVD/" // MX4SIO su mass1

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
    DIR *dir = opendir(GAMES_PREFIX);
    if (dir == NULL) dir = opendir("mass0:/DVD/"); // Fallback
    
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
    // Il README dice: neutrino.elf -bsd=mx4sio -dvd=mass:path/to/filename.iso
    // Nota: Neutrino usa spesso 'mass:' internamente per riferirsi al bsd selezionato
    sprintf(iso_full_path, "mass:/DVD/%s", isoName); 

    char *args[6];
    args[0] = NEUTRINO_PATH;
    args[1] = "-bsd=mx4sio"; // Parametro corretto dal README
    args[2] = (char*)malloc(256);
    sprintf(args[2], "-dvd=%s", iso_full_path); // Formato corretto: -dvd=percorso
    args[3] = "-qb"; // Quick boot per saltare il logo
    args[4] = NULL;

    scr_clear();
    scr_printf("AVVIO NEUTRINO v1.5+...\n");
    scr_printf("Driver: MX4SIO\n");
    scr_printf("ISO: %s\n", isoName);

    SifExitRpc();
    FlushCache(0);
    FlushCache(2);

    LoadExecPS2(NEUTRINO_PATH, 4, args);
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
            scr_printf("NEUTRINO LAUNCHER v2.0 (MODULAR)\n");
            scr_printf("======================================\n\n");
            
            if (gameCount == 0) {
                scr_printf("GIOCHI NON TROVATI IN %s\n", GAMES_PREFIX);
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
    }
    return 0;
}
