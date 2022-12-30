#include "LaConsola.h"

LaTelevision tv;

static const uint32_t coloresNorm[] = {
        0x757575, 0x8f1b27, 0xab0000, 0x9f0047, 0x77008f, 0x1300ab, 0x0000a7, 0x000b7f,
        0x002f43, 0x004700, 0x005100, 0x173f00, 0x5f3f1b, 0x000000, 0x000000, 0x000000,
        0xbcbcbc, 0xef7300, 0xef3b23, 0xf30083, 0xbf00bf, 0x5b00e7, 0x002bdb, 0x0f4fcb,
        0x00738b, 0x009700, 0x00ab00, 0x3b9300, 0x8b8300, 0x000000, 0x000000, 0x000000,
        0xffffff, 0xffbf3f, 0xff975f, 0xfd8ba7, 0xff7bf7, 0xb777ff, 0x6377ff, 0x3b9bff,
        0x3fbff3, 0x13d383, 0x4bdf4f, 0x98f858, 0xdbeb00, 0x787878, 0x000000, 0x000000,
        0xffffff, 0xffe7ab, 0xffd7c7, 0xffcbd7, 0xffc7ff, 0xdbc7ff, 0xb3bfff, 0xabdbff,
        0xa3e7ff, 0xa3ffe3, 0xbff3ab, 0xcfffb3, 0xf3ff9f, 0xc4c4c4, 0x000000, 0x000000

};

static const int32_t coloresRG[] = { // balores de paleta fceux
        0x343434, 0x00023C, 0x000051, 0x0d004a, 0x2e0031, 0x410003, 0x3e0000, 0x270000,
        0x0a0600, 0x001100, 0x001900, 0x001200, 0x180e1a, 0x000000, 0x000000, 0x000000,
        0x676767, 0x002e81, 0x0b1386, 0x3a0087, 0x5e0065, 0x770029, 0x700c00, 0x631c0c,
        0x3c3100, 0x004600, 0x005300, 0x004511, 0x003b41, 0x000000, 0x000000, 0x000000,
        0x969696, 0x1d6695, 0x304da9, 0x7145a8, 0x8c3b95, 0x933a65, 0x923b30, 0x905115,
        0x85680e, 0x427504, 0x247d1d, 0x278d4d, 0x00817a, 0x2c2c2c, 0x000000, 0x000000,
        0x969696, 0x5f8496, 0x71799e, 0x7b729d, 0x946f95, 0x946f7d, 0x946b63, 0x937c5c,
        0x928457, 0x809358, 0x5e8c68, 0x629374, 0x57928d, 0x6d6d6d, 0x000000, 0x000000
};

static const int32_t coloresGri[] = {
        0x747474, 0x747474, 0x747474, 0x747474, 0x747474, 0x747474, 0x747474, 0x747474,
        0x747474, 0x747474, 0x747474, 0x747474, 0x747474, 0x747474, 0x747474, 0x747474,
        0xBCBCBC, 0xBCBCBC, 0xBCBCBC, 0xBCBCBC,	0xBCBCBC, 0xBCBCBC, 0xBCBCBC, 0xBCBCBC,
        0xBCBCBC, 0xBCBCBC, 0xBCBCBC, 0xBCBCBC, 0xBCBCBC, 0xBCBCBC, 0xBCBCBC, 0xBCBCBC,
        0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC,
        0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC,
        0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC,
        0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC, 0xFCFCFC
};

static const int32_t paletteMirrors[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0,
        16, 17, 18, 4, 19, 20, 21, 8, 22, 23, 24, 12, 25, 26, 27
};

bool LaTelevision_Carga() {
    bool elSuceso = true;
    if(
        LaConsola_CargaMemoriaS32(&tv.coloresNormal, PALETTE_SIZE) &
        LaConsola_CargaMemoriaS32(&tv.coloresGris, PALETTE_SIZE) &
        LaConsola_CargaMemoriaS32(&tv.coloresRGB, PALETTE_SIZE) &
        LaConsola_CargaMemoriaS32(&tv.telaBuffer, SCREEN_RES_X * SCREEN_RES_Y) &
        LaConsola_CargaMemoriaS32(&tv.paletasPtr, 32)
    ) {}
    else {
        elSuceso = false;
        printf("Error cargando memoria, HAY QUE COMPRAR COMPUTADORA NUEVA, BAGARTO HUEVON !!!\n");
    }

    for(int n =0; n < 64; n++) {
        tv.coloresNormal[n] = coloresNorm[n];
        tv.coloresGris[n] = coloresGri[n];
        tv.coloresRGB[n] = coloresRG[n];
    }

    for(int n =0; n < 32; n++)
        tv.paletasPtr[n] = paletteMirrors[n];

    tv.laPaletaActual = tv.coloresNormal;

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) !=0) {
        elSuceso = false;
        printf("Error cargando graficos, TIENES UNA MIERDA DE VIDEO DISPLAY!!!\n");

    }
    tv.yanela = SDL_CreateWindow("XiruStation v 1.0", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 448, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );
    tv.render = SDL_CreateRenderer(tv.yanela, -1, 0);
    tv.testura = SDL_CreateTexture(tv.render, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, 256, 224);
    SDL_RenderSetScale(tv.render, 2, 2);
    return elSuceso;
}

void LaTelevision_DeseniaScanline(int scanlineNum) {
for (int m = 0; m < SCREEN_RES_X; m++)
    tv.telaBuffer[tv.telaBufferCursor++] = tv.laPaletaActual[ppu.palettes[tv.paletasPtr[ppu.drawBuffer[(scanlineNum * SCREEN_RES_X) + m] & 0x1F]]];
}

void LaTelevision_DeseniaFrame() {
    while( SDL_PollEvent( &tv.elEvento ) != 0 ) {
        if(tv.elEvento.type == SDL_KEYDOWN)
            LosControls_PresionaBoton(&tv.elEvento);
        else if(tv.elEvento.type == SDL_KEYUP)
            LosControls_SoltaBoton(&tv.elEvento);
        else if( tv.elEvento.type == SDL_QUIT )
            consola.emulationActive = false;
    }
    SDL_UpdateTexture(tv.testura, NULL, tv.telaBuffer, SCREEN_RES_X * sizeof(int32_t));
    SDL_RenderClear(tv.render);
    SDL_RenderCopy(tv.render, tv.testura, NULL, NULL);
    SDL_RenderPresent(tv.render);

//    ppu.tiempoFinal = SDL_GetTicks() - ppu.tiempoInicial;
  //  if (ppu.tiempoFinal < 16)
    //    SDL_Delay(16 - ppu.tiempoFinal);
   // ppu.tiempoInicial = SDL_GetTicks();

    tv.telaBufferCursor = 0;
}

void LaTelevision_setPaletaGris() {
    tv.laPaletaActual = tv.coloresGris;
}

void LaTelevision_setPaletaNormal() {
    tv.laPaletaActual = tv.coloresNormal;
}

void LaTelevision_setPaletaRGB() {
    tv.laPaletaActual = tv.coloresRGB;
}

void LaTelevision_Descarga() {
    free(tv.telaBuffer);
    free(tv.coloresNormal);
    free(tv.coloresGris);
    free(tv.coloresRGB);
    free(tv.paletasPtr);
    SDL_DestroyTexture(tv.testura);
	SDL_DestroyRenderer(tv.render);
	SDL_DestroyWindow(tv.yanela);
	SDL_Quit();
	exit(0);
}

