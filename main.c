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
int currentDevice = 0; // 0=USB, 1=MX4SIO
int force_redraw = 1;

static char padBuf[256] __attribute__((aligned(64)));

const char* deviceNames[] = { "USB (Mass Storage)", "MX4SIO (SD Card)" };
const char* neutrinoArg[]  = { "-bs=mass", "-bs=sd" };

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
    // Usiamo "mass:" per entrambi perché uLaunchELF/SDK vedono entrambi lì
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
    // Percorso ISO per Neutrino
    sprintf(iso_full_path, "mass:/DVD/%s", isoName);

    char *elf_path = "mass:/neutrino.elf";
    char *args[6];
    args[0] = elf_path;
    args[1] = (char*)neutrinoArg[currentDevice]; 
    args[2] = "-mod=dvd";             
    args[3] = iso_full_path;          
    args[4] = NULL;

    scr_clear();
    scr_printf("AVVIO IN CORSO...\n");
    scr_printf("File: %s\n", elf_path);
    
    // Esegue Neutrino
    execv(elf_path, args);
    
    // Se fallisce
    scr_setfontcolor(0x0000FF);
    scr_printf("\nERRORE: Non trovo mass:/neutrino.elf\n");
    scr_printf("Verifica che il file sia nella root della USB/SD\n");
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
        if (force_redraw) {
            scr_clear();
            scr_setfontcolor(0x00FFFF);
            scr_printf("NEUTRINO LAUNCHER v1.4\n");
            scr_printf("======================\n\n");
            
            scr_setfontcolor(0xFFFFFF);
            scr_printf("MODALITA': %s (Premi L1/R1 per cambiare)\n", deviceNames[currentDevice]);
            scr_printf("GIOCHI TROVATI: %d\n\n", gameCount);

            if (gameCount == 0) {
                scr_printf(" < NESSUN GIOCO TROVATO IN mass:/DVD/ >\n");
            } else {
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
                if(gameCount > 0) { selectedIndex = (selectedIndex + 1) % gameCount; force_redraw = 1; }
            }
            if ((new_pad & PAD_UP) && !(old_pad & PAD_UP)) {
                if(gameCount > 0) { selectedIndex = (selectedIndex - 1 + gameCount) % gameCount; force_redraw = 1; }
            }
            if ((new_pad & PAD_R1) && !(old_pad & PAD_R1)) {
                currentDevice = 1; // MX4SIO
                force_redraw = 1;
            }
            if ((new_pad & PAD_L1) && !(old_pad & PAD_L1)) {
                currentDevice = 0; // USB
                force_redraw = 1;
            }
            if ((new_pad & PAD_CROSS) && !(old_pad & PAD_CROSS)) {
                if (gameCount > 0) launch_neutrino(gameList[selectedIndex]);
            }
            old_pad = new_pad;
        }
        for(int i=0; i<100000; i++) asm("nop");
    }
    return 0;
}
