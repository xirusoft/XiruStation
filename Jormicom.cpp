#include "LaConsola.h"

int main(int argc, char* args[]) {

    int sucesio = -1;

    char fullPath[50];
    char laExtension[] = ".nes";
    sprintf(fullPath, "%s%s", args[1], laExtension);

    if(LaConsola_Liga(fullPath)) {
        printf("WOOOOOOOOOOOOOOOOOOOWWWWWWWWWWWWWW JUEGA CON EL SUPER PODER DE JORO !!!!\n");
        LaConsola_StartEmulacion();
        sucesio = 0;
        LaConsola_Desliga();
    }
    else {
        printf("Que malo, amiguito. Hubo un error !\n");
        LaConsola_Desliga();
    }

    return sucesio;
}
