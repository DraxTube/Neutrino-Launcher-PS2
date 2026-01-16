#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

#define NEUTRINO_PATH "mass:/neutrino.elf"
#define GAMES_PREFIX  "mass1:/DVD/" 

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
    if (dir == NULL) dir = opendir("mass0:/DVD/");
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
    char iso_arg[256];
    sprintf(iso_arg, "-dvd=mass:/DVD/%s", isoName); 

    char *args[10];
    args[0] = NEUTRINO_PATH;
    args[1] = "-bsd=mx4sio"; 
    args[2] = "-bsdfs=fatfs"; // Usiamo fatfs come indicato nei tuoi moduli
    args[3] = iso_arg;
    args[4] = "-cwd=mass:/";  // Obbligatorio per trovare /modules/
    args[5] = "-qb";
    args[6] = "-dbc";         // Attiva colori di debug
    args[7] = "-logo";        // Necessario per stabilità v1.7.0
    args[8] = NULL;

    scr_clear();
    scr_printf("AVVIO NEUTRINO v1.7.0...\n");
    scr_printf("DRIVER: MX4SIO (SLOT 2)\n");
    scr_printf("ISO: %s\n", isoName);

    // Disabilita tutto prima di cedere il controllo a Neutrino
    padPortClose(0,0);
    SifExitRpc();
    FlushCache(0);
    FlushCache(2);

    LoadExecPS2(NEUTRINO_PATH, 8, args);
}

int main() {
    init_system();
    struct padButtonStatus buttons;
    u32 old_pad = 0, new_pad = 0;
    scan_games();

    while(1) {
        if (force_redraw) {
            scr_clear();
            scr_setfontcolor(0x00FFFF);
            scr_printf("NEUTRINO LAUNCHER v2.5\n");
            scr_printf("==============================\n\n");
            if (gameCount == 0) scr_printf("ISO NON TROVATE SU MX4SIO!\n");
            else {
                for(int i = 0; i < gameCount; i++) {
                    if (i == selectedIndex) { scr_setfontcolor(0x00FF00); scr_printf(" > %s\n", gameList[i]); }
                    else { scr_setfontcolor(0xAAAAAA); scr_printf("   %s\n", gameList[i]); }
                }
            }
            force_redraw = 0;
        }
        if (padRead(0, 0, &buttons) != 0) {
            new_pad = 0xffff ^ buttons.btns;
            if ((new_pad & PAD_DOWN) && !(old_pad & PAD_DOWN)) { if (selectedIndex < gameCount - 1) { selectedIndex++; force_redraw = 1; } }
            if ((new_pad & PAD_UP) && !(old_pad & PAD_UP)) { if (selectedIndex > 0) { selectedIndex--; force_redraw = 1; } }
            if ((new_pad & PAD_CROSS) && !(old_pad & PAD_CROSS)) launch_neutrino(gameList[selectedIndex]);
            old_pad = new_pad;
        }
    }
    return 0;
}
