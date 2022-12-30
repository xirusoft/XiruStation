#ifndef _MICRO_GENIUS
#define _MICRO_GENIUS

#include "LaConsola.h"
#include <SDL2/SDL.h>

#define SCREEN_RES_X 256
#define SCREEN_RES_Y 224
#define PALETTE_SIZE 64

struct LaTelevision {
    int32_t *coloresNormal;
    int32_t *coloresGris;
    int32_t *coloresRGB;
    int32_t *laPaletaActual;
    int32_t *telaBuffer;
    int32_t *paletasPtr;
    int32_t telaBufferCursor;

    SDL_Event elEvento;
    SDL_Window *yanela;
    SDL_Renderer *render;
    SDL_Texture *testura;
};

extern LaTelevision tv;
bool LaTelevision_Carga();
void LaTelevision_setPaletaGris();
void LaTelevision_setPaletaNormal();
void LaTelevision_setPaletaRGB();
void LaTelevision_DeseniaScanline(int scanlineNum);
void LaTelevision_DeseniaFrame();
void LaTelevision_Descarga();

#endif
