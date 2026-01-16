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
    // Specifichiamo il percorso completo incluso il device per evitare ambiguità
    sprintf(iso_arg, "-dvd=mass:/DVD/%s", isoName); 

    char *args[8];
    args[0] = NEUTRINO_PATH;
    args[1] = "-bsd=mx4sio"; 
    args[2] = "-bsdfs=exfat"; // FORZIAMO exFAT come visto nel tuo toml
    args[3] = iso_arg;
    args[4] = "-cwd=mass:/";  // Forza la cartella di lavoro sulla USB per trovare i moduli
    args[5] = "-qb";
    args[6] = "-dbc";         // Attiva colori di debug: se vedi colori prima del crash, sappiamo dove fallisce
    args[7] = NULL;

    scr_clear();
    scr_printf("LANCIO AVANZATO v2.4...\n");
    
    // Se la tua SD è FAT32, cambia args[2] in "-bsdfs=fatfs"
    
    padPortClose(0,0);
    SifExitRpc();
    FlushCache(0);
    FlushCache(2);

    LoadExecPS2(NEUTRINO_PATH, 7, args);
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
            scr_printf("NEUTRINO LAUNCHER v2.4 - DEBUG MODE\n");
            scr_printf("======================================\n\n");
            if (gameCount == 0) scr_printf("ISO NON TROVATE!\n");
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
            if ((new_pad & PAD_CROSS) && !(old_pad & PAD_CROSS)) { if (gameCount > 0) launch_neutrino(gameList[selectedIndex]); }
            old_pad = new_pad;
        }
    }
    return 0;
}
