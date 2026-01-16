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
#include <limits.h> // Per PATH_MAX di sistema

// --- CONFIGURAZIONE ---
#define MAX_GAMES 100

// Strutture per lo stato
char gameList[MAX_GAMES][64];
int gameCount = 0;
int selectedIndex = 0;
int currentDevice = 0; // 0=USB, 1=MX4SIO, 2=HDD, 3=UDPBD

// Buffer Controller
static char padBuf[256] __attribute__((aligned(64)));

// Mappa dispositivi e parametri Neutrino
const char* deviceNames[] = { "USB (Mass Storage)", "MX4SIO (SD Card)", "HDD (Internal)", "UDPBD (Ethernet)" };
const char* devicePrefix[] = { "mass:", "mass:", "hdd0:", "mass:" }; 
const char* neutrinoArg[]  = { "-bs=mass", "-bs=sd", "-bs=ata", "-bs=nbd" };

// --- FUNZIONI SISTEMA ---
void init_system() {
    SifInitRpc(0);
    init_scr(); // Schermo testuale
    scr_clear();
    
    // Patch vitali per lanciare altri ELF
    sbv_patch_enable_lmb();
    sbv_patch_disable_prefix_check();

    // Inizializza PAD
    padInit(0);
    padPortOpen(0, 0, padBuf);
}

// Scansiona la cartella DVD del dispositivo selezionato
void scan_games() {
    gameCount = 0;
    selectedIndex = 0;
    char path[PATH_MAX];
    
    sprintf(path, "%s/DVD/", devicePrefix[currentDevice]);
    
    scr_printf("Scansione in corso su %s...\n", path);

    DIR *dir = opendir(path);
    if (dir != NULL) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL && gameCount < MAX_GAMES) {
            if (strstr(ent->d_name, ".ISO") || strstr(ent->d_name, ".iso")) {
                strncpy(gameList[gameCount], ent->d_name, 63);
                gameCount++;
            }
        }
        closedir(dir);
    } else {
        gameCount = 0; 
    }
}

void launch_neutrino(char *isoName) {
    char elf_path[] = "mass:/neutrino.elf"; 
    char iso_full_path[PATH_MAX];
    
    sprintf(iso_full_path, "%s/DVD/%s", devicePrefix[currentDevice], isoName);

    char *args[6];
    args[0] = elf_path;               
    args[1] = (char*)neutrinoArg[currentDevice]; 
    args[2] = "-mod=dvd";             
    args[3] = iso_full_path;          
    args[4] = NULL;

    scr_clear();
    scr_printf("LANCIO IN CORSO...\n");
    scr_printf("Neutrino: %s\n", elf_path);
    scr_printf("Driver: %s\n", args[1]);
    scr_printf("ISO: %s\n", iso_full_path);
    
    execv(elf_path, args);
    
    scr_printf("\nERRORE: Impossibile avviare neutrino.elf\nControlla la USB.");
    sleep(5);
}

int main() {
    init_system();
    
    struct padButtonStatus buttons;
    u32 old_pad = 0;
    u32 new_pad = 0;

    scan_games();

    while(1) {
        scr_clear();
        scr_setfontcolor(0xFFFFFF); // Bianco (Corretto senza _)
        scr_printf("== NEUTRINO UNIVERSAL LAUNCHER ==\n\n");
        
        scr_printf("Sorgente: [ %s ] (Premi L1/R1 per cambiare)\n", deviceNames[currentDevice]);
        scr_printf("----------------------------------------\n");

        if (gameCount == 0) {
            scr_printf("   < NESSUN GIOCO TROVATO >\n");
            scr_printf("   Assicurati che i giochi siano in /DVD/\n");
        } else {
            for(int i=0; i<gameCount; i++) {
                if (i == selectedIndex) {
                    scr_setfontcolor(0x00FF00); // Verde (Corretto senza _)
                    scr_printf(" > %s\n", gameList[i]);
                } else {
                    scr_setfontcolor(0xAAAAAA); // Grigio (Corretto senza _)
                    scr_printf("   %s\n", gameList[i]);
                }
            }
        }

        if (padRead(0, 0, &buttons) != 0) {
            new_pad = 0xffff ^ buttons.btns;

            if ((new_pad & PAD_DOWN) && !(old_pad & PAD_DOWN)) {
                if(gameCount > 0) selectedIndex = (selectedIndex + 1) % gameCount;
            }
            if ((new_pad & PAD_UP) && !(old_pad & PAD_UP)) {
                if(gameCount > 0) selectedIndex = (selectedIndex - 1 + gameCount) % gameCount;
            }
            if ((new_pad & PAD_R1) && !(old_pad & PAD_R1)) {
                currentDevice = (currentDevice + 1) % 4;
                scan_games();
            }
            if ((new_pad & PAD_L1) && !(old_pad & PAD_L1)) {
                currentDevice = (currentDevice - 1 + 4) % 4;
                scan_games();
            }
            if ((new_pad & PAD_CROSS) && !(old_pad & PAD_CROSS)) {
                if (gameCount > 0) launch_neutrino(gameList[selectedIndex]);
            }
            if ((new_pad & PAD_TRIANGLE) && !(old_pad & PAD_TRIANGLE)) {
                scan_games();
            }
            old_pad = new_pad;
        }

        // Delay anti-flicker
        for(int i=0; i<300000; i++) asm("nop"); 
    }

    return 0;
}
