#include "LaConsola.h"

LaConsola consola;

static const int scanlineCycles[] = { 114, 114, 113 };

bool LaConsola_Liga(char* nombreRom) {
    bool elSuceso;

    if( ElCartujo_Carga(nombreRom) &
        LaTelevision_Carga() &
        ElMapperBase_Carga()&
        LaCpu_Carga() &
        LaPpu_Carga() &
        LaApu_Carga()
    )
        elSuceso = true;
    else {
        printf("ERROR al ligar la Consola, tirar la computadora a la basura !!!\n");
        elSuceso = false;
    }

    consola.emulationActive = true;
    printf("prg %d chr %d mir %d mapper %d\n", ElCartujo_GetNumPrg(), ElCartujo_GetNumChr(), ElCartujo_GetMirror(), ElCartujo_GetMapperNum());
    return elSuceso;
}

void LaConsola_Desliga() {
    ElCartujo_Descarga();
    LaCpu_Descarga();
    LaPpu_Descarga();
    LaApu_Descarga();
    ElMapperBase_Descarga();
}

 void LaConsola_StartEmulacion() {
     int cyclesToUpdate = 0;

     while(consola.emulationActive) {
        consola.scanline = 0;
        while (consola.scanline < 262) { // ejecuta un frame
            if ( (consola.scanline >= 0) && (consola.scanline < 240)) { // visible

                if (ppu.sprRenderEnabled)
                    LaPpu_RenderSprites(consola.scanline);

                if(ppu.bgRenderEnabled)
                    LaPpu_UpdateBgLine(consola.scanline);

            }
            LaPpu_RenderScanline(ppu.ciclosSobrando , consola.scanline);

            while (LaCpu_GetCurrentCycles() < scanlineCycles[consola.scanlineCycleIndex]) {

                cyclesToUpdate = LaCpu_ExecuteInstruction();
                cpu.totalCiclos+=cyclesToUpdate;
                LaPpu_RenderScanline(cyclesToUpdate*3 , consola.scanline);

                mapper.update(cyclesToUpdate, consola.scanline);

            }
            LaApu_AudioUpdate();
            LaCpu_UpdateCurrentCycles(-scanlineCycles[consola.scanlineCycleIndex]);
            if((consola.scanline > 7) && (consola.scanline < 232))
                LaTelevision_DeseniaScanline(consola.scanline-8);
            consola.scanline++;
              if(++consola.scanlineCycleIndex > 2)
                consola.scanlineCycleIndex = 0;
        }
        LaApu_audioTocaFrame();
        LaTelevision_DeseniaFrame();
        LaPpu_ResetRenderBuffer();
        consola.frameCount++;
     }
}


bool LaConsola_CargaMemoriaS16(int16_t** elArray, size_t elSize) {
    *elArray = (int16_t*)malloc(sizeof(int16_t)*elSize);

    if(elArray != NULL)
        return true;
    else
        return false;

}


bool LaConsola_CargaMemoriaS32(int32_t** elArray, size_t elSize) {
    *elArray = (int32_t*)malloc(sizeof(int32_t)*elSize);

    if(elArray != NULL)
        return true;
    else
        return false;

}

bool LaConsola_CargaMemoriaF(float** elArray, size_t elSize) {
    *elArray = (float*)malloc(sizeof(float)*elSize);

    if(elArray != NULL)
        return true;
    else
        return false;

}

bool LaConsola_CargaMemoriaB(bool** elArray, size_t elSize) {
    *elArray = (bool*)malloc(sizeof(bool)*elSize);
    if(elArray != NULL)
        return true;
    else
        return false;
}

int32_t LaConsola_GetScanline() {
    return consola.scanline;
}

bool LaConsola_IsVBlankPeriod() {
    if(consola.scanline > 240 && consola.scanline < 261)
        return true;
    else
        return false;
}
