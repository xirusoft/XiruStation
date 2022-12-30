#ifndef _HI_TOP_GAME
#define _HI_TOP_GAME
#include <cstdint>
#include <SDL2/SDL.h>
#include "LaConsola.h"

#define BUTTON_UP 0x10
#define BUTTON_DOWN 0x20
#define BUTTON_LEFT 0x40
#define BUTTON_RIGHT 0x80
#define BUTTON_A 1
#define BUTTON_B 2
#define BUTTON_SELECT 4
#define BUTTON_START 8

struct LosControls
{
    int32_t p1Strobe;        	// Para input, hay que escribir 1 y despues 0 en 4016 para latch control state // todos controls
    int32_t p2Strobe;
    int32_t readCounter; 		// # de la lectura me diz de cual boton quieres el input
    int32_t buttonFlags;        // Flags de state de boton, 1: pressed, 2: released

};

void LosControls_PresionaBoton(SDL_Event*);
void LosControls_SoltaBoton(SDL_Event*);
int32_t LosControls_Reg4016Read();
void LosControls_Reg4016Write(int32_t);
void LosControls_Reg4017Write(int32_t);

#endif
