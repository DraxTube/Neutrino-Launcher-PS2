#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <stdio.h>
#include <debug.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <libpad.h>

// Buffer per il pad (allineato a 64 byte per il DMA)
static char padBuf[256] __attribute__((aligned(64)));
char iso_list[20][256]; 
int iso_count = 0;

void load_modules(void) {
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:PADMAN", 0, NULL);
}

void wait_pad_ready(int port, int slot) {
    int state;
    // Correzione: Usiamo PAD_STATE_STABLE invece di PAD_STATE_READY
    while((state = padGetState(port, slot)) != PAD_STATE_STABLE) {
        if(state == PAD_STATE_DISCONN) scr_printf("Pad non connesso...\n");
    }
}

void scan_iso() {
    DIR *d = opendir("mass:/DVD/");
    struct dirent *dir;
    iso_count = 0;
    if (d) {
        while ((dir = readdir(d)) != NULL && iso_count < 20) {
            if (strstr(dir->d_name, ".ISO") || strstr(dir->d_name, ".iso")) {
                strncpy(iso_list[iso_count], dir->d_name, 256);
                iso_count++;
            }
        }
        closedir(d);
    }
}

void start_neutrino(const char *iso_name) {
    char *neutrino_path = "mass:/neutrino.elf";
    char full_iso_path[512];
    sprintf(full_iso_path, "mass:/DVD/%s", iso_name);

    char *args[5];
    args[0] = neutrino_path;
    args[1] = "-bs=mass"; 
    args[2] = "-mod=dvd";
    args[3] = full_iso_path;
    args[4] = NULL;

    scr_printf("\nAvvio di Neutrino in corso...\n");
    execv(neutrino_path, args);
}

int main() {
    SifInitRpc(0);
    load_modules();
    init_scr();

    scr_printf("==========================================\n");
    scr_printf("      NEUTRINO LAUNCHER - DraxTube        \n");
    scr_printf("   GitHub: github.com/DraxTube            \n");
    scr_printf("   YouTube: youtube.com/@DraxTube01       \n");
    scr_printf("==========================================\n\n");

    padInit(0);
    padPortOpen(0, 0, padBuf);
    wait_pad_ready(0, 0);

    scr_printf("Scansione mass:/DVD/ in corso...\n");
    scan_iso();

    if (iso_count == 0) {
        scr_printf("ERRORE: Nessuna ISO trovata in mass:/DVD/\n");
        while(1);
    }

    int selected = 0;
    struct padButtonStatus buttons;
    u32 paddata, old_pad = 0, new_pad;

    while(1) {
        scr_clear();
        scr_printf("Sviluppato da DraxTube - Seleziona un gioco:\n");
        scr_printf("------------------------------------------\n");
        for(int i=0; i<iso_count; i++) {
            scr_printf("%s %s\n", (i == selected) ? " >" : "  ", iso_list[i]);
        }
        scr_printf("\n[SU/GIU] Naviga  [X] Avvia gioco");

        if(padRead(0, 0, &buttons) != 0) {
            paddata = 0xffff ^ buttons.btns;
            new_pad = paddata & ~old_pad;
            old_pad = paddata;

            if(new_pad & PAD_DOWN) selected = (selected + 1) % iso_count;
            if(new_pad & PAD_UP)   selected = (selected - 1 + iso_count) % iso_count;
            if(new_pad & PAD_CROSS) break;
        }
        usleep(30000); 
    }

    start_neutrino(iso_list[selected]);

    scr_printf("\nERRORE: Impossibile avviare mass:/neutrino.elf\n");
    while(1);

    return 0;
}
