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

    // Prepariamo gli argomenti (usiamo mass:/ perché i giochi li vede lì)
    char *args[] = {
        "mass:/neutrino.elf",
        "-bs=mass", 
        "-mod=dvd",
        iso_path,
        NULL
    };

    scr_clear();
    scr_printf("INIZIALIZZAZIONE LANCIO...\n");
    scr_printf("Gioco: %s\n", isoName);

    // TENTATIVI DI LANCIO A TAPPETO
    // 1. Percorso standard minuscolo
    execv("mass:/neutrino.elf", args);
    
    // 2. Percorso standard MAIUSCOLO (spesso risolve su PS2)
    args[0] = "mass:/NEUTRINO.ELF";
    execv("mass:/NEUTRINO.ELF", args);
    
    // 3. Variante mass0: (usata da alcuni driver)
    args[0] = "mass0:/neutrino.elf";
    execv("mass0:/neutrino.elf", args);
    
    // 4. Variante mass0: MAIUSCOLO
    args[0] = "mass0:/NEUTRINO.ELF";
    execv("mass0:/NEUTRINO.ELF", args);

    // Se arriviamo qui, nessuno dei tentativi ha funzionato
    scr_setfontcolor(0x0000FF); // Rosso
    scr_printf("\nERRORE FATALE: neutrino.elf non trovato.\n");
    scr_printf("Verifica che il file sia nella ROOT della USB\n");
    scr_printf("e che si chiami esattamente neutrino.elf\n");
    scr_printf("\nPremi un tasto per tornare al menu...");
    
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
            scr_printf("NEUTRINO LAUNCHER v1.5 - FIX LANCIO\n");
            scr_printf("====================================\n\n");
            
            if (gameCount == 0) {
                scr_printf("ERRORE: Cartella mass:/DVD/ vuota o non trovata!\n");
            } else {
                scr_setfontcolor(0xFFFFFF);
                scr_printf("Seleziona Gioco (X per avviare):\n\n");
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
