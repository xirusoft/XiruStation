#ifndef _DENDY
#define _DENDY

#include "LaConsola.h"
#include <cstdint>

#define PRG_BYTE 4
#define CHR_BYTE 5
#define MAPPER_BYTE 6
#define HEADER_SIZE 16
#define BANK_16K 16384
#define BANK_8K 8192

struct ElCartujo {
	uint8_t* romHeader;
	uint8_t* prg;
	uint8_t* chr;
};

extern ElCartujo cartujo;

bool ElCartujo_Carga(char*);
uint8_t ElCartujo_GetNumPrg();
uint8_t ElCartujo_GetNumChr();
uint8_t ElCartujo_GetMapperNum();
uint8_t ElCartujo_GetMirror();
bool ElCartujo_Is4ScreenNt();
uint8_t ElCartujo_PrgRead(uint32_t addr);
void ElCartujo_ChrWrite(uint32_t addr, uint8_t value);
uint8_t ElCartujo_ChrRead(uint32_t addr);
void ElCartujo_Descarga();

#endif
