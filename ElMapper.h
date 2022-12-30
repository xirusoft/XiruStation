#ifndef _PHANTOM_SYSTEM
#define _PHANTOM_SYSTEM

#include "LaConsola.h"
#include <cstdint>

#define INTERRUPT_NMI 8
#define INTERRUPT_FRAME_IRQ 1
#define INTERRUPT_DMC_IRQ 2
#define INTERRUPT_MAPPER_IRQ 4
#define OFFSET_1K 1024
#define OFFSET_8K 8192
#define OFFSET_16K 16384

struct ElMapper {
    void (*ioWrite)(int32_t addr, int32_t value);
    int32_t (*ioRead)(int32_t addr);
    int32_t (*vramRead)(int32_t addr);
    void (*vramWrite)(int32_t addr, int32_t value);
    void (*update)(int, int);
    int32_t *systemRam;
    uint8_t interruptSignal;
    bool nmiDelayFlag;
    bool irqDelayFlag;
    bool updateDmcCycles;
    bool updateDmaCycles;
    bool nmiEnable;
    uint8_t reg2003;
    int32_t reg2000;
    int32_t reg2006;
    int32_t reg4014;
    int32_t bufferReg2007;
    int32_t addrIncrementer;
    bool isSecondWrite;
    int32_t nt0Offset, nt1Offset, nt2Offset, nt3Offset;
    int32_t *nameTableData;
    int32_t patternTable0Offset, patternTable1Offset;
    int32_t prgBank0Ptr, prgBank1Ptr;
    int32_t chrMask;
    int32_t prgMask;

    // Mapper MMC3
    int32_t prg8KBank0Ptr;			// PRG Bank 8000-9FFF
	int32_t prg8KBank1Ptr;			// PRG Bank A000-BFFF
	int32_t prg8KBank2Ptr;			// PRG Bank C000-DFFF
	int32_t prg8KBank3Ptr;			// PRG Bank E000-FFFF
	int32_t swappableBank;			// D6 de $8000
	int32_t reg8000Command;			// Codigo de D0-2 de $8000
	int32_t irqCounter;			// Counter Register C000
	int32_t irqLatch;			// balor de recarga register C001
	bool irqRecargar;
	bool irqEnable;		// Enable MMC3 IRQ ($E000)
	int32_t swappableChr;			// D7 de $8000
	int32_t *patternTable1KOffset;	// los Offsets para cada 1K de pattern
	int32_t numPrg;
	int32_t lastScanline;
	int32_t *wRam;

	// Mapper 023
    int32_t prgSwapBit;
	int32_t irqLoopFlag;
	int32_t irqPreScaler;
	int32_t irqType;
	int32_t irqReload;
	int32_t* pattern1KBankNum;

	// mapper 001
    int32_t writeCounter;
	int32_t shiftRegister;
	int32_t prgSwitchSize;		// Bit 3 del register. 0: switch todo 32k prg, 1: switch 16K
	int32_t prgSwitchBank;		// Bit 2 del Register. 0: switch 16K de C000; 1: switch 16K de 8000
	int32_t chrBankMode;
	int32_t numChr;
	int32_t fourMegaCartBit;


};

extern ElMapper mapper;

bool ElMapperBase_Carga();
int32_t ElMapper_GetRstAddr();
int32_t ElMapper_GetNmiAddr();
int32_t ElMapper_GetIrqAddr();

uint8_t ElMapper_GetInterruptSignal();
void ElMapper_SetInterruptSignal(uint8_t);
void ElMapper_SetNmiDelayFlag(bool);
bool ElMapper_GetNmiDelayFlag();
void ElMapper_ClearInterruptSignal(uint8_t);
bool ElMapper_IrqIsRequested();
void ElMapper_SetIrqDelayFlag(bool);
bool ElMapper_GetIrqDelayFlag();
bool ElMapper_GetNmiEnableFlag();
void ElMapper_SetReg2003(int32_t);
int32_t ElMapperBase_IoRead(int32_t addr);
void ElMapperBase_IoWrite(int addr, int value);
void ElMapperBase_Descarga();
void ElMapperBase_VramWrite(int addr, int value);
int ElMapperBase_VramRead(int addr);
void ElMapperBase_Update(int losCiclos, int scanlineNum);
void ElMapperBase_Setup();
void ElMapper_CargaAtributos() ;
void ElMapper002_Setup();
void ElMapper002_IoWrite(int addr, int balor);
void ElMapper003_IoWrite(int, int);
void ElMapper003_Setup();
void ElMapper004_Setup();
void ElMapper004_IoWrite(int addr, int balor);
int ElMapper004_IoRead(int addr);
int ElMapper004_VramRead(int addr);
void ElMapper004_VramWrite(int addr, int elBalor);
void ElMapper004_Update(int losCiclos, int scanlineNum);
void ElMapper023_IoWrite(int addr, int balor);
int ElMapper023_IoRead(int addr);
void ElMapper023_Setup();

void ElMapper001_Setup();

#endif
