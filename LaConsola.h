#ifndef _POLYSTATION
#define _POLYSTATION

#include "ElCartujo.h"
#include "ElMapper.h"
#include "LaCpu.h"
#include "LaApu.h"
#include "LaTelevision.h"
#include "LosControls.h"
#include "LaPpu.h"
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

struct LaConsola {
	bool emulationActive;
	int16_t scanline;
    int scanlineCycleIndex;
    int frameCount;
};

extern LaConsola consola;

bool LaConsola_Liga(char*);
void LaConsola_Desliga();
bool LaConsola_CargaMemoriaS8(int8_t, size_t);
bool LaConsola_CargaMemoriaS16(int16_t**, size_t);
bool LaConsola_CargaMemoriaS32(int32_t**, size_t);
bool LaConsola_CargaMemoriaF(float**, size_t);
bool LaConsola_CargaMemoriaB(bool**, size_t);
int32_t LaConsola_GetScanline();
bool LaConsola_IsVBlankPeriod();
void LaConsola_StartEmulacion();

#endif
