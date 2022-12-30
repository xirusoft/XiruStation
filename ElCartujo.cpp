#include "LaConsola.h"

ElCartujo cartujo;

bool ElCartujo_Carga(char* nombreRom) {

    FILE *fdp = fopen(nombreRom, "rb");
    bool elSuceso = true;

    if(fdp == NULL) {
        printf("Error abrindo el fichero de ROM!\n");
        elSuceso = false;
    }
    else {
        cartujo.romHeader = (uint8_t*)malloc(sizeof(uint8_t)*HEADER_SIZE);

        if(cartujo.romHeader == NULL) {
            printf("Error en alocacion de Header, computadora de MIERDA!\n");
            elSuceso = false;
        }
        else
            fread(cartujo.romHeader, 1, HEADER_SIZE, fdp);

        int prgSize = ElCartujo_GetNumPrg() * BANK_16K;
        cartujo.prg = (uint8_t*)malloc(sizeof(uint8_t)* prgSize);

        if(cartujo.prg == NULL) {
            printf("Error en alocacion de PRG, computadora de BUESTA!\n");
            elSuceso = false;
        }
        else
            fread(cartujo.prg, 1, prgSize, fdp);

        int chrSize = ElCartujo_GetNumChr() * BANK_8K;

        if(chrSize == 0)
            chrSize = BANK_8K * 2;

        cartujo.chr = (uint8_t*)malloc(sizeof(uint8_t)* chrSize);

        if(cartujo.chr == NULL) {
            printf("Error en alocacion de CHR, computadora de CARAJO!\n");
            elSuceso = false;
        }
        else
            fread(cartujo.chr, 1, chrSize, fdp);

        fclose(fdp);
    }

    return elSuceso;
}

uint8_t ElCartujo_GetNumPrg() {
  return cartujo.romHeader[PRG_BYTE];
}

uint8_t ElCartujo_GetNumChr() {
  return cartujo.romHeader[CHR_BYTE];
}

uint8_t ElCartujo_GetMapperNum() {
  return (cartujo.romHeader[MAPPER_BYTE] >> 4) | (cartujo.romHeader[MAPPER_BYTE + 1] & 0xF0);
}

uint8_t ElCartujo_GetMirror() {
  return cartujo.romHeader[MAPPER_BYTE] & 1;
}

bool ElCartujo_Is4ScreenNt() {
  return (cartujo.romHeader[MAPPER_BYTE] & 8) == 8 ? true : false;
}

uint8_t ElCartujo_PrgRead(uint32_t addr) {
  return cartujo.prg[addr];
}

void ElCartujo_ChrWrite(uint32_t addr, uint8_t value) {
  cartujo.chr[addr] = value;
}

uint8_t ElCartujo_ChrRead(uint32_t addr) {
  return cartujo.chr[addr];
}

void ElCartujo_Descarga() {
  free(cartujo.romHeader);
  free(cartujo.prg);
  free(cartujo.chr);
}


