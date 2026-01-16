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

// DEFINIZIONI PERCORSI
#define NEUTRINO_PATH "mass:/neutrino.elf"
// Il launcher legge da mass1 (MX4SIO), ma dice a Neutrino di usare mass:
#define LAUNCHER_READ_PATH "mass1:/DVD/" 

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
    // Prova mass1 (MX4SIO)
    DIR *dir = opendir(LAUNCHER_READ_PATH);
    // Se fallisce, prova mass0
    if (dir == NULL) dir = opendir("mass0:/DVD/");
    
    if (dir != NULL) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL && gameCount < MAX_GAMES) {
            // Filtra solo file ISO
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
    
    // Costruiamo il percorso come se fossimo dentro Neutrino
    // Neutrino v1.7.0 monta l'MX4SIO come dispositivo 'mass:' principale
    sprintf(iso_arg, "-dvd=mass:/DVD/%s", isoName); 

    char *args[10];
    args[0] = NEUTRINO_PATH;
    args[1] = "-bsd=mx4sio";  // Driver hardware
    args[2] = "-bsdfs=exfat"; // Profilo filesystem (ORA PUNTA AL FILE GIUSTO GRAZIE ALLA TUA MODIFICA)
    args[3] = iso_arg;        // Percorso ISO
    args[4] = "-cwd=mass:/";  // Directory di lavoro (USB)
    args[5] = "-qb";          // Quick Boot
    args[6] = "-dbc";         // Debug Colors
    args[7] = NULL;

    scr_clear();
    scr_printf("AVVIO FINAL FIX v2.8...\n");
    scr_printf("1. Driver MX4SIO attivato\n");
    scr_printf("2. Profilo exFAT attivato (tramite bdmfs_fatfs.irx)\n");
    scr_printf("3. ISO: %s\n", isoName);
    
    // Spegnimento totale dei servizi prima del lancio
    padPortClose(0,0);
    SifExitRpc();
    FlushCache(0);
    FlushCache(2);

    // Esecuzione
    LoadExecPS2(NEUTRINO_PATH, 7, args);
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
            scr_printf("NEUTRINO LAUNCHER v2.8 (TOML FIXED)\n");
            scr_printf("=======================================\n\n");
            
            if (gameCount == 0) {
                scr_printf("NESSUNA ISO TROVATA!\n");
                scr_printf("Verifica: SD nello Slot 2? Cartella DVD esiste?\n");
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
        // Piccolo delay per non saturare la CPU
        for(int i=0; i<30000; i++) asm("nop");
    }
    return 0;
}
