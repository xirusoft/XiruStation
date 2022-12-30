#include "LaConsola.h"

ElMapper mapper;



uint8_t ElMapper_GetInterruptSignal() {
    return mapper.interruptSignal;
}

void ElMapper_SetInterruptSignal(uint8_t losflagsRegister) {
    mapper.interruptSignal |= losflagsRegister;
}

void ElMapper_ClearInterruptSignal(uint8_t losflagsRegister) {
    mapper.interruptSignal = mapper.interruptSignal & (~losflagsRegister) & 0xff;
}

void ElMapper_SetNmiDelayFlag(bool value) {
    mapper.nmiDelayFlag = value;
}

bool ElMapper_GetNmiDelayFlag() {
    return mapper.nmiDelayFlag;
}

bool ElMapper_IrqIsRequested() {

    if((mapper.interruptSignal & 7) !=0)
        return true;
    else
        return false;

}

void ElMapper_SetIrqDelayFlag(bool value) {
    mapper.irqDelayFlag = value;
}

bool ElMapper_GetIrqDelayFlag() {
    return mapper.irqDelayFlag;
}

int32_t ElMapper_GetRstAddr() {
    return mapper.ioRead(0xFFFC) | (mapper.ioRead(0xFFFD) << 8);
}

int32_t ElMapper_GetIrqAddr() {
    return mapper.ioRead(0xFFFE) | (mapper.ioRead(0xFFFF) << 8);
}

int32_t ElMapper_GetNmiAddr() {
    return mapper.ioRead(0xFFFA) | (mapper.ioRead(0xFFFB) << 8);
}

bool ElMapper_GetNmiEnableFlag() {
    return mapper.nmiEnable;
}

void ElMapper_SetReg2003(int32_t elBalor) {
    mapper.reg2003 = elBalor;
}

bool ElMapperBase_Carga() {
    if(LaConsola_CargaMemoriaS32(&mapper.systemRam, 0x800) == false) {
        printf("Error cargando Mapper, tu computadora es una MIERDA DE VACUNO !!\n");
        return false;
    }
    mapper.nt0Offset = 0;
    if(ElCartujo_Is4ScreenNt()) {
        if(LaConsola_CargaMemoriaS32(&mapper.nameTableData, 4096) == false) {
            printf("Error cargando Mapper, tu computadora es una MIERDA DE VACUNO !!\n");
            return false;
        }
        mapper.nt1Offset = OFFSET_1K;
        mapper.nt2Offset = mapper.nt1Offset + OFFSET_1K;
        mapper.nt3Offset = mapper.nt2Offset + OFFSET_1K;
    }
    else {
        size_t ntSiz;
        if(ElCartujo_GetMapperNum() == 5)
            ntSiz = 4096;
        else
            ntSiz = 2048;
        if(LaConsola_CargaMemoriaS32(&mapper.nameTableData, ntSiz) == false) {
            printf("Error cargando Mapper, tu computadora es una MIERDA DE VACUNO !!\n");
            return false;
        }
        if (ElCartujo_GetMirror() == 0) {
            mapper.nt1Offset = 0;
            mapper.nt2Offset = OFFSET_1K;
        }
        else {
            mapper.nt1Offset = OFFSET_1K;
            mapper.nt2Offset = 0;
        }
        mapper.nt3Offset = OFFSET_1K;
    }
    mapper.patternTable0Offset = 0;
    mapper.patternTable1Offset = 4096;
    mapper.prgBank0Ptr = 0;
    if(ElCartujo_GetNumPrg() == 1)
        mapper.prgBank1Ptr = 0;
    else
        mapper.prgBank1Ptr = OFFSET_16K;
    ElMapper_CargaAtributos();
    return true;
}

void ElMapperBase_Descarga() {
    free(mapper.nameTableData);
    free(mapper.systemRam);
}

int32_t ElMapperBase_IoRead(int32_t addr) {
    if(addr < 0x2000)
        return mapper.systemRam[addr & 0x7ff];
    else if ( (addr >= 0x2000) && (addr < 0x4000) ) { // PPU Registers
        switch (addr % 8) {
            case 0:
                return (mapper.reg2000 & 0xfc) | ((LaPpu_GetLoopyV() >> 10) & 3);
            case 1:
                return 0xff;
            case 2: {
                int32_t temp = LaPpu_GetFlagsRegister();
                LaPpu_TurnOffFlags(NMI_FLAG);
                mapper.isSecondWrite = false;
                // ACA SI LECTURA DE 2002 OCURRE EN EL MOMENTO DE NMI, SUPRIME LA INTERRUPCION
                int32_t laSoma =  LaPpu_GetLineDot() + (LaCpu_GetCurrentOpCycles()*3);
                if( LaConsola_GetScanline() == 240 && (laSoma > 341))
                    LaPpu_SetSuprimeNmiFlag(true);
                return temp;
            }
            case 3:
                return mapper.reg2003;
            case 4:
                return LaPpu_SprOamRead(mapper.reg2003);
            case 5:
                return 0;
            case 6:
                return mapper.reg2006;
            case 7:
                int32_t tempor;
                int32_t lv = LaPpu_GetLoopyV();
                if (lv >= 0x3F00)
                    tempor = LaPpu_PalettesRead(tv.paletasPtr[lv & 0x1F]);
                else {
                    tempor = mapper.bufferReg2007;
                    LaPpu_SetVramCaller(VRAM_READ_CALLER_SPR);
                    mapper.bufferReg2007 = mapper.vramRead(lv);
                }
                int32_t currScanline = LaConsola_GetScanline();
                if( LaPpu_IsRenderActive() && ( ( (currScanline < 240)) || (currScanline == 261)) ) {
                    LaPpu_UpdateCoarseX();
                    LaPpu_UpdateCoarseY();
                }
                else
                    LaPpu_SetLoopyV((lv + mapper.addrIncrementer) & 0x7FFF);
                return tempor;
        }
    }
    else if(addr == 0x4015)
        return LaApu_Reg4015Read();
    else if (addr == 0x4016)// INPUT 1 READ
        return LosControls_Reg4016Read();
    else if(addr == 0x4017)   // hay que retornar 0x40 para informar que control 2 esta conectado y nada fue pressed
        return 0x40;
    else if ((addr >= 0x8000) && (addr < 0xC000))
        return ElCartujo_PrgRead(addr - 0x8000 + mapper.prgBank0Ptr);
    else if ((addr >= 0xC000) && (addr < 0x10000))
        return ElCartujo_PrgRead(addr - 0xC000 + mapper.prgBank1Ptr);
    else
        return 0;
}

void ElMapperBase_IoWrite(int addr, int value) {
    if (addr < 0x2000)
        mapper.systemRam[addr & 0x7ff] = value;
    else if ((addr >= 0x2000) && (addr < 0x4000)) {	// PPU Registers
        switch (addr % 8) {
            case 0:// REGISTER $2000
                if(LaConsola_IsVBlankPeriod() && value > 0x7f && LaPpu_FlagIsSet(NMI_FLAG) && mapper.nmiEnable == false) {
                    ElMapper_SetNmiDelayFlag(true);
                    ElMapper_SetInterruptSignal(INTERRUPT_NMI);
                }
                mapper.reg2000 = value;
                mapper.nmiEnable = (value & 0x80) == 0x80 ? true : false;
                mapper.addrIncrementer = (value & 4) == 4 ? 32 : 1;
                LaPpu_Reg2000Write(value);
                break;
            case 1:// REGISTER 2001
                switch((value & 0xe0)>>5) {	// color intensity bits
                    case 0:
                        LaTelevision_setPaletaNormal();
                        break;
                    case 7:
                        LaTelevision_setPaletaGris();
                        break;
                }
                if ((value & 1) == 1)
                    LaTelevision_setPaletaGris();
                LaPpu_Reg2001Write(value);
                break;
            case 3:// REGISTER 2003
                mapper.reg2003 = value;
                break;
            case 4: // REGISTER 2004
                LaPpu_SprOamWrite(mapper.reg2003, value);
                mapper.reg2003 = (mapper.reg2003 + 1) & 0xff; // OAMADDR INCREMENTED DESPUES DE ESCRIPTA PERO NO LECTURA
                break;
            case 5: { // REGISTER 2005
                int32_t lt1 = LaPpu_GetLoopyT();
                if (mapper.isSecondWrite == false) {    // LA 1ER ESCRIPTA DEFINE LOS HORIZONTAL LATCHES
                    LaPpu_SetLoopyT((lt1 & 0x7fe0) | ((value & 0xf8) >> 3));
                    LaPpu_SetFineX(value & 7);
                    mapper.isSecondWrite = true;
                }
                else {   // LA 2ER ESCRIPTA DEVINE VERTICAL LATCHES Y RESET EL 2ER WRITE LATCH
                    LaPpu_SetLoopyT((lt1 & 0xc1f) | ((value & 7) << 12) | ((value & 0xc0) << 2) | ((value & 0x38) << 2));
                    mapper.isSecondWrite = false;
                }
                break;
            }
            case 6: {// REGISTER 2006
                mapper.reg2006 = value;
                int32_t lt2 = LaPpu_GetLoopyT();
                if (mapper.isSecondWrite == false) {
                    LaPpu_SetLoopyT((lt2 & 0xff) | ((value & 0x3f) << 8));
                    mapper.isSecondWrite = true;
                }
                else {   // DESPUES DE LA 2ER ESCRIPTA ACTUALIZA LOS REGISTRADORES REALES
                    if(LaPpu_GetLineDot() == 255 || LaPpu_GetLineDot() == 256)
                        ppu.fue255 = true;
                    LaPpu_SetLoopyT((lt2 & 0x7f00) | value);
                    LaPpu_SetLoopyV(LaPpu_GetLoopyT());
                    mapper.isSecondWrite = false;

                }
                break;
            }
            case 7:// REGISTER 2007
                int32_t lv = LaPpu_GetLoopyV();
                mapper.vramWrite(lv, value);
                int32_t currScanline = LaConsola_GetScanline();
                if( LaPpu_IsRenderActive() && ( ( (currScanline < 240)) || (currScanline == 261)) ) {
                        LaPpu_UpdateCoarseX();
                        LaPpu_UpdateCoarseY();
                }
                else
                    LaPpu_SetLoopyV((lv + mapper.addrIncrementer) & 0x7FFF);
                break;
        }
    }
    else if (addr == 0x4000) // APU REGISTER 4000
        LaApu_SqrRegister0Write(SQUARE1, value);
    else if (addr == 0x4001)
        LaApu_SqrRegister1Write(SQUARE1, value);
    else if (addr == 0x4002)
        LaApu_SqrRegister2Write(SQUARE1, value);
    else if (addr == 0x4003)
        LaApu_SqrRegister3Write(SQUARE1, value);
    else if (addr == 0x4004)
        LaApu_SqrRegister0Write(SQUARE2, value);
    else if (addr == 0x4005)
        LaApu_SqrRegister1Write(SQUARE2, value);
    else if (addr == 0x4006)
        LaApu_SqrRegister2Write(SQUARE2, value);
    else if (addr == 0x4007)
        LaApu_SqrRegister3Write(SQUARE2, value);
    else if (addr == 0x4008)
        LaApu_TrgRegister0Write(value);
    else if (addr == 0x400A)
        LaApu_TrgRegister1Write(value);
    else if (addr == 0x400B)
        LaApu_TrgRegister2Write(value);
    else if (addr == 0x400C)
        LaApu_NoiseRegister0Write(value);
    else if (addr == 0x400E)
        LaApu_NoiseRegister1Write(value);
    else if (addr == 0x400F)
        LaApu_NoiseRegister2Write(value);
    else if (addr == 0x4010)
        LaApu_DmcRegister0Write(value);
    else if (addr == 0x4011)
        LaApu_DmcRegister1Write(value);
    else if (addr == 0x4012)
        LaApu_DmcRegister2Write(value);
    else if (addr == 0x4013)
        LaApu_DmcRegister3Write(value);
    else if (addr == 0x4014) {    // SPRITZ DMA
        mapper.reg4014 = value * 0x100;
        for (int n = 0; n < 0x100; n++)
           LaPpu_SprOamWrite((mapper.reg2003 + n) & 0xFF, mapper.ioRead(mapper.reg4014 + n));
        mapper.updateDmaCycles = true;
    }
    else if (addr == 0x4015) {
        LaApu_ToggleChannel(value);
            apu.dmcIrqFlag = 0;
            ElMapper_ClearInterruptSignal(INTERRUPT_DMC_IRQ);
    }
    else if (addr == 0x4016)  // LATCH CONTROL 1
        LosControls_Reg4016Write(value);
    else if (addr == 0x4017) {   // LATCH CONTROL 2
        LosControls_Reg4017Write(value);
        LaApu_Reg4017Write(value);
    }
}

void ElMapperBase_VramWrite(int addr, int value) {
    if (addr < 0x1000) {
        if(ElCartujo_GetNumChr() == 0) // SOLO PUEDE ESCRIBIR EN CHR SI CHR RAM
            ElCartujo_ChrWrite((addr%4096) + mapper.patternTable0Offset, value);
    }
    else if ((addr >= 0x1000) && (addr < 0x2000)) {
        if(ElCartujo_GetNumChr() == 0) // SOLO PUEDE ESCRIBIR EN CHR SI CHR RAM
            ElCartujo_ChrWrite((addr%4096) + mapper.patternTable1Offset, value);
    }
    else if ((addr >= 0x2000) && (addr < 0x2400))
        mapper.nameTableData[addr - 0x2000 + mapper.nt0Offset] = value;
    else if ((addr >= 0x2400) && (addr < 0x2800))
        mapper.nameTableData[addr - 0x2400 + mapper.nt1Offset] = value;
    else if ((addr >= 0x2800) && (addr < 0x2C00))
        mapper.nameTableData[addr - 0x2800 + mapper.nt2Offset] = value;
    else if ((addr >= 0x2C00) && (addr < 0x3000))
        mapper.nameTableData[addr - 0x2C00 + mapper.nt3Offset] = value;
    else if ((addr >= 0x3000) && (addr < 0x3400))
        mapper.nameTableData[addr - 0x3000 + mapper.nt0Offset] = value;
    else if ((addr >= 0x3400) && (addr < 0x3800))
        mapper.nameTableData[addr - 0x3400 + mapper.nt1Offset] = value;
    else if ((addr >= 0x3800) && (addr < 0x3C00))
        mapper.nameTableData[addr - 0x3800 + mapper.nt2Offset] = value;
    else if ((addr >= 0x3C00) && (addr < 0x3F00))
        mapper.nameTableData[addr - 0x3C00 + mapper.nt3Offset] = value;
    else if ((addr >= 0x3F00) && (addr < 0x4000)) {
        LaPpu_PalettesWrite(tv.paletasPtr[addr & 0x1F], value & 0x3f);
    }
}

 int ElMapperBase_VramRead(int addr) {
    if(addr < 0x1000)
        return ElCartujo_ChrRead((addr%4096) + mapper.patternTable0Offset);
    else if ((addr >= 0x1000) && (addr < 0x2000))
        return ElCartujo_ChrRead((addr%4096) + mapper.patternTable1Offset);
    else if ((addr >= 0x2000) && (addr < 0x2400))
        return mapper.nameTableData[addr - 0x2000 + mapper.nt0Offset];
    else if ((addr >= 0x2400) && (addr < 0x2800))
        return mapper.nameTableData[addr - 0x2400 + mapper.nt1Offset];
    else if ((addr >= 0x2800) && (addr < 0x2C00))
        return mapper.nameTableData[addr - 0x2800 + mapper.nt2Offset];
    else if ((addr >= 0x2C00) && (addr < 0x3000))
        return mapper.nameTableData[addr - 0x2C00 + mapper.nt3Offset];
    else if ((addr >= 0x3000) && (addr < 0x3400))
        return mapper.nameTableData[addr - 0x3000 + mapper.nt0Offset];
    else if ((addr >= 0x3400) && (addr < 0x3800))
        return mapper.nameTableData[addr - 0x3400 + mapper.nt1Offset];
    else if ((addr >= 0x3800) && (addr < 0x3C00))
        return mapper.nameTableData[addr - 0x3800 + mapper.nt2Offset];
    else if ((addr >= 0x3C00) && (addr < 0x3F00))
        return mapper.nameTableData[addr - 0x3C00 + mapper.nt3Offset];
    else if ((addr >= 0x3F00) && (addr < 0x4000))
        return LaPpu_PalettesRead(tv.paletasPtr[addr & 0x1F]);
    else
        return 0xff;
}

void ElMapperBase_Update(int losCiclos, int scanlineNum) {	// para MMC, VRC y octros
}

void ElMapperBase_Setup() {
    mapper.ioRead = ElMapperBase_IoRead;
    mapper.ioWrite = ElMapperBase_IoWrite;
    mapper.vramRead = ElMapperBase_VramRead;
    mapper.vramWrite = ElMapperBase_VramWrite;
    mapper.update = ElMapperBase_Update;
}

int ElMapper001_IoRead(int addr) {
    if ((addr >= 0x6000) && (addr < 0x8000))
        return mapper.wRam[addr - 0x6000];
    else
        return ElMapperBase_IoRead(addr);
}

void ElMapper001_IoWrite(int addr, int balor) {
    if ((addr >= 0x6000) && (addr < 0x8000))
        mapper.wRam[addr - 0x6000] = balor;
    else if ((addr >= 0x8000) && (addr < 0x10000)) {
        if((balor & 0x80) == 0x80) {	// Reset registers
            mapper.shiftRegister = mapper.writeCounter = 0;
            mapper.prgSwitchSize = 1;
            mapper.prgSwitchBank = 1;
        }
        else {
            mapper.shiftRegister = (mapper.shiftRegister & (0xFF - (1 << mapper.writeCounter))) | ((balor & 1) << mapper.writeCounter);
            mapper.writeCounter++;
            if(mapper.writeCounter == 5) {
                if ((addr >= 0x8000) && (addr < 0xA000)) {	//Control Register
                    switch(mapper.shiftRegister & 3) {	// Mirror orientation
                        case 0:	// 0 = One screen, all mirrors lower bank
                            mapper.nt0Offset = mapper.nt1Offset = mapper.nt2Offset = mapper.nt3Offset = 0;
                            break;
                        case 1:	// 1 = One screen, all mirrors upper bank
                            mapper.nt0Offset = mapper.nt1Offset = mapper.nt2Offset = mapper.nt3Offset = 1024;
                            break;
                        case 2:	// 2 = Mirror Vertical
                            mapper.nt1Offset = mapper.nt3Offset = 1024;
                            mapper.nt0Offset = mapper.nt2Offset = 0;
                            break;
                        case 3:	// 3 = Mirror Horizontal
                            mapper.nt1Offset = mapper.nt0Offset = 0;
                            mapper.nt2Offset = mapper.nt3Offset = 1024;
                            break;
                    }
                    mapper.prgSwitchSize = (mapper.shiftRegister >>3) & 1;
                    mapper.prgSwitchBank = (mapper.shiftRegister >>2) & 1;
                    mapper.chrBankMode = ((mapper.shiftRegister & 0x10) >> 4) & 1;
                }
                else if ((addr >= 0xA000) && (addr < 0xC000)) {		//CHR Bank 0 Register
                    mapper.prgBank1Ptr = (mapper.prgBank1Ptr & 0x3ffff) |  ((mapper.shiftRegister & mapper.fourMegaCartBit) << 14);
                    mapper.prgBank0Ptr = (mapper.prgBank0Ptr & 0x3ffff) |  ((mapper.shiftRegister & mapper.fourMegaCartBit) << 14);
                    if(mapper.chrBankMode == 0) {	// si switch todo el 8K de VROM
                        mapper.patternTable0Offset = 4096 * (mapper.shiftRegister & (mapper.chrMask & 0x1e));
                        mapper.patternTable1Offset = mapper.patternTable0Offset + 4096;
                    }
                    else // o solamente los primeros 4K de Pattern Table 0
                        mapper.patternTable0Offset = 4096 * (mapper.shiftRegister & mapper.chrMask);
                }
                if ((addr >= 0xC000) && (addr < 0xE000)) {			// CHR Bank 1 Register
                    if(mapper.chrBankMode != 0) {	// si switch todo el 8K de VROM, IGNORA LA ESCRIPTA
                        mapper.patternTable1Offset = 4096 * (mapper.shiftRegister &  mapper.chrMask);
                    }
                }
                if ((addr >= 0xE000) && (addr < 0x10000)) {			// PRG Bank Register
                    if(mapper.prgSwitchSize == 0) {// Switch todo el 32K, Shift Right 1 casita los bits 0-3 de register
                        mapper.prgBank0Ptr = ((mapper.shiftRegister & 0xF) >> 1) * 32768;
                        mapper.prgBank1Ptr = mapper.prgBank0Ptr + OFFSET_16K;
                    }
                    else {	// switch solamente uno de los 16K Banks
                        if(mapper.prgSwitchBank == 0) {// Bank 0 en 0x8000 y cambia los 16K de 0xC000
                            mapper.prgBank0Ptr = mapper.prgBank0Ptr & 0x40000;
                            mapper.prgBank1Ptr = (mapper.prgBank1Ptr & 0x40000) | ((mapper.shiftRegister & mapper.prgMask) * OFFSET_16K);
                        }
                        else { // Last Bank en 0xC000 y cambia los 16K de 0x8000
                            mapper.prgBank1Ptr = (mapper.prgBank1Ptr & 0x40000) | (((mapper.numPrg -1) & mapper.prgMask) * OFFSET_16K);
                            mapper.prgBank0Ptr = (mapper.prgBank0Ptr & 0x40000) | ((mapper.shiftRegister & mapper.prgMask) * OFFSET_16K);
                        }
                    }
                }
                mapper.writeCounter = mapper.shiftRegister = 0;
            }
        }
    }
    else
        ElMapperBase_IoWrite(addr, balor);
}

void ElMapper001_Setup() {
    mapper.numPrg = ElCartujo_GetNumPrg();
    mapper.numChr = ElCartujo_GetNumChr();
    LaConsola_CargaMemoriaS32(&mapper.wRam, 0x2000);
    if( mapper.numPrg > 16) {
        mapper.prgBank1Ptr = ((mapper.numPrg/2) -1) * OFFSET_16K;
        mapper.prgMask = 15;
        mapper.fourMegaCartBit = 0x10;
    }
    else {
        mapper.prgBank1Ptr = (mapper.numPrg -1) * OFFSET_16K;
        mapper.prgMask = (mapper.numPrg -1);
        mapper.fourMegaCartBit = 0;
    }
    mapper.prgSwitchSize = 1;
    mapper.prgSwitchBank = 1;
    if(mapper.numChr == 0)
        mapper.chrMask = 1 ;
    else
        mapper.chrMask = (mapper.numChr * 2) -1;

    mapper.ioRead = ElMapper001_IoRead;
    mapper.ioWrite = ElMapper001_IoWrite;
    mapper.vramRead = ElMapperBase_VramRead;
    mapper.vramWrite = ElMapperBase_VramWrite;
    mapper.update = ElMapperBase_Update;
}

void ElMapper002_IoWrite(int addr, int balor) {
    if ((addr >= 0x8000) && (addr < 0x10000))
        mapper.prgBank0Ptr = OFFSET_16K * (balor & mapper.prgMask);
    else
        ElMapperBase_IoWrite(addr, balor);
}

void ElMapper002_Setup() {
    mapper.chrMask = ElCartujo_GetNumChr() - 1;
    mapper.prgMask = ElCartujo_GetNumPrg() -1;
    mapper.prgBank1Ptr = (ElCartujo_GetNumPrg() -1) * OFFSET_16K;
    mapper.ioRead = ElMapperBase_IoRead;
    mapper.ioWrite = ElMapper002_IoWrite;
    mapper.vramRead = ElMapperBase_VramRead;
    mapper.vramWrite = ElMapperBase_VramWrite;
    mapper.update = ElMapperBase_Update;
}

void ElMapper003_IoWrite(int addr, int balor) {
    if ((addr >= 0x8000) && (addr < 0x10000)) {
        mapper.patternTable0Offset = OFFSET_8K * (balor & mapper.chrMask);
        mapper.patternTable1Offset = mapper.patternTable0Offset + 4096;
}
else
    ElMapperBase_IoWrite(addr, balor);
}

void ElMapper003_Setup() {
    mapper.chrMask = ElCartujo_GetNumChr() - 1;
    mapper.ioRead = ElMapperBase_IoRead;
    mapper.ioWrite = ElMapper003_IoWrite;
    mapper.vramRead = ElMapperBase_VramRead;
    mapper.vramWrite = ElMapperBase_VramWrite;
    mapper.update = ElMapperBase_Update;
}

void ElMapper004_IoWrite(int addr, int balor) {
    if ((addr >= 0x6000) && (addr < 0x8000))
        mapper.wRam[addr - 0x6000] = balor;
    else if (addr >= 0x8000 && addr <0x9000 && addr %2 == 0) {			// Register 8000 = Bank Select
        mapper.swappableBank = balor & 0x40;
        mapper.swappableChr = balor & 0x80;
        mapper.reg8000Command = balor & 7;
        if(mapper.swappableBank == 0)	// 8000-9FFF swapable, C000-DFFF fixo al penultimo banco
            mapper.prg8KBank2Ptr = (mapper.numPrg -1) * OFFSET_16K;
        else if(mapper.swappableBank == 0x40)	// C000-DFFF swapable, 8000-9FFF fixo al penultimo banco
            mapper.prg8KBank0Ptr = (mapper.numPrg -1) * OFFSET_16K;
    }

    else if (addr >= 0x8000 && addr <0x9000 && addr %2 != 0) {		// Register 8001 = Bank Data
        switch(mapper.reg8000Command) {
            case 0:
            case 1:		// Swap 2K CHR en $0 o en $1000 de acuerdo con D7 de 8000
            //	System.out.printf("aca com %x swap %x\n", reg8000Command, swappableBank);
                if(mapper.swappableChr == 0) {
                    mapper.patternTable1KOffset[mapper.reg8000Command*2] = (balor & mapper.chrMask) * OFFSET_1K;
                    mapper.patternTable1KOffset[(mapper.reg8000Command*2)+1] = ((balor & mapper.chrMask) * OFFSET_1K) + OFFSET_1K;
                }
                else if(mapper.swappableChr == 0x80) {
                    mapper.patternTable1KOffset[(mapper.reg8000Command*2)+4] = (balor & mapper.chrMask) * OFFSET_1K;
                    mapper.patternTable1KOffset[(mapper.reg8000Command*2)+5] = ((balor & mapper.chrMask) * OFFSET_1K) + OFFSET_1K;
                }
                break;
            case 2:	// Swap 1K Chr
            case 3:
            case 4:
            case 5:
                if(mapper.swappableChr == 0x80)	// Swap bancos 1K Chr en 0 - FFF
                    mapper.patternTable1KOffset[mapper.reg8000Command-2] = (balor & mapper.chrMask) * OFFSET_1K;
                if(mapper.swappableChr == 0)	// Swap bancos 1K Chr en 0 - FFF
                    mapper.patternTable1KOffset[mapper.reg8000Command+2] = (balor & mapper.chrMask) * OFFSET_1K;
                break;
            case 6:	// Select 8K Bank de $8000 o de $C000
                if(mapper.swappableBank == 0)
                    mapper.prg8KBank0Ptr = (balor & mapper.prgMask) * OFFSET_8K;
                else if(mapper.swappableBank == 0x40)
                    mapper.prg8KBank2Ptr = (balor & mapper.prgMask) * OFFSET_8K;
                break;
            case 7: // Select 8K Bank de $A000
                mapper.prg8KBank1Ptr = (balor & mapper.prgMask) * OFFSET_8K;
                break;
        }
    }
    else if (addr == 0xA000) {	// name table mirroring
        if(ElCartujo_Is4ScreenNt())
            return;
        else {
            if ((balor & 1)  == 1) {// horizontal
                mapper.nt1Offset = 0;
                mapper.nt2Offset = OFFSET_1K;
            }
            else {
                mapper.nt1Offset = OFFSET_1K;
                mapper.nt2Offset = 0;
            }
        }
    }
    else if (addr == 0xA001) {	// SRAM PROTECT
        // EL SENIOR NESTON ME DIZ QUE LOS EMULADORES NO IMPLEMENTAN PARA NO CONFLICTAR CON MMC6
    }
    else if (addr == 0xC000)
        mapper.irqLatch = balor;
    else if (addr == 0xC001) {
        mapper.irqCounter = 0;
        mapper.irqRecargar = true ;
    }
    else if (addr == 0xE000) {
        mapper.irqEnable = false;
        ElMapper_ClearInterruptSignal(INTERRUPT_MAPPER_IRQ);
    }
    else if (addr == 0xE001)
        mapper.irqEnable = true;
    else
        ElMapperBase_IoWrite(addr, balor);
}

int ElMapper004_VramRead(int addr) {
    if(addr < 0x2000)
        return ElCartujo_ChrRead((addr%OFFSET_1K)+ mapper.patternTable1KOffset[((int)addr/0x400)]);
    else
        return ElMapperBase_VramRead(addr);
}

void ElMapper004_VramWrite(int addr, int elBalor) {
    if (addr < 0x2000) {
        if(ElCartujo_GetNumChr() == 0) // SOLO PUEDE ESCRIBIR EN CHR SI CHR RAM
            ElCartujo_ChrWrite((addr%OFFSET_1K)+ mapper.patternTable1KOffset[ ((int)addr/0x400)], elBalor);
    }
    else
        ElMapperBase_VramWrite(addr, elBalor);
}

int ElMapper004_IoRead(int addr) {
    if ((addr >= 0x6000) && (addr < 0x8000))
        return mapper.wRam[addr - 0x6000];
    else if ((addr >= 0x8000) && (addr < 0xA000))
        return ElCartujo_PrgRead(addr - 0x8000 + mapper.prg8KBank0Ptr);
    else if ((addr >= 0xA000) && (addr < 0xC000))
        return ElCartujo_PrgRead(addr - 0xA000 + mapper.prg8KBank1Ptr);
    else if ((addr >= 0xC000) && (addr < 0xE000))
        return ElCartujo_PrgRead(addr - 0xC000 + mapper.prg8KBank2Ptr);
    else if ((addr >= 0xE000) && (addr < 0x10000))
        return ElCartujo_PrgRead(addr - 0xE000 + mapper.prg8KBank3Ptr);
    else
        return ElMapperBase_IoRead(addr);
}

void ElMapper004_Update(int losCiclos, int scanlineNum) {
    if(  (( (scanlineNum < 240)  ) || (scanlineNum == 261) ) && (scanlineNum != mapper.lastScanline) && ppu.lineDot >= 270) {
        mapper.lastScanline = scanlineNum ;
        if (LaPpu_IsRenderActive()) {
            if(mapper.irqCounter == 0 || mapper.irqRecargar == true) {
                mapper.irqCounter = mapper.irqLatch;
                mapper.irqRecargar = false;
            }
            else
                mapper.irqCounter--;
            if((mapper.irqCounter == 0) && mapper.irqEnable ) {
                //setIrqDelayFlag(false);
                ElMapper_SetInterruptSignal(INTERRUPT_MAPPER_IRQ);
            }
        }
    }
}

void ElMapper004_Setup() {
    mapper.numPrg = ElCartujo_GetNumPrg();
    mapper.chrMask = (ElCartujo_GetNumChr() * 8) -1;
    mapper.prgMask = (mapper.numPrg * 2) -1;
    mapper.prg8KBank0Ptr = 0;
    mapper.prg8KBank1Ptr = OFFSET_8K;
    mapper.prg8KBank2Ptr = (mapper.numPrg -1) * OFFSET_16K;
    mapper.prg8KBank3Ptr = mapper.prg8KBank2Ptr + OFFSET_8K;
    mapper.swappableBank = 0;
    mapper.reg8000Command = 0;
    mapper.swappableChr = 0;
    mapper.irqEnable = false;
    LaConsola_CargaMemoriaS32(&mapper.patternTable1KOffset, 8);
    LaConsola_CargaMemoriaS32(&mapper.wRam, 0x2000);
    for(int n = 0; n< 8; n++)
        mapper.patternTable1KOffset[n] = n*OFFSET_1K;
    mapper.ioWrite = ElMapper004_IoWrite;
    mapper.ioRead = ElMapper004_IoRead;
    mapper.vramRead = ElMapper004_VramRead;
    mapper.vramWrite = ElMapper004_VramWrite;
    mapper.update = ElMapper004_Update;

}

void ElMapper023_IoWrite(int addr, int balor) {
    if ((addr >= 0x6000) && (addr < 0x8000))
        mapper.wRam[addr - 0x6000] = balor;
    else if (addr >= 0x8000 && addr < 0x9000) {
        if(mapper.prgSwapBit == 0)
            mapper.prg8KBank0Ptr = (balor & mapper.prgMask) * 8192;
        else
            mapper.prg8KBank2Ptr = (balor & mapper.prgMask) * 8192;
    }
    else if (addr == 0x9000) {
        if ((balor & 3)  == 1) {// horizontal
            mapper.nt0Offset = mapper.nt1Offset = 0;
            mapper.nt2Offset = mapper.nt3Offset = 1024;
        }
        else if ((balor & 3)  == 0){ // vertical
            mapper.nt1Offset = mapper.nt3Offset = 1024;
            mapper.nt0Offset = mapper.nt2Offset = 0;
        }
        else if ((balor & 3)  == 2) // 1 screen low bank
            mapper.nt1Offset = mapper.nt0Offset = mapper.nt2Offset = mapper.nt3Offset = 0;
        else if ((balor & 3)  == 3) // 1 screen high bank
            mapper.nt1Offset = mapper.nt0Offset = mapper.nt2Offset = mapper.nt3Offset = 1024;
    }
    else if (addr == 0x9001 || addr == 0x9004) {
        mapper.prgSwapBit = balor & 1;
        if(mapper.prgSwapBit == 0)
            mapper.prg8KBank2Ptr = (ElCartujo_GetNumPrg() -1) * 16384;
        else
            mapper.prg8KBank0Ptr = (ElCartujo_GetNumPrg() -1) * 16384;
    }
    else if (addr >= 0xA000 && addr < 0xB000)
        mapper.prg8KBank1Ptr = (balor & mapper.prgMask) * 8192;
    /* LA POSICION DE LOS REGISTERS VARIA CFE SUBTIPO DE VRC4 - VER WIKI:
     * EG VRC4b ES X000, X002... VRC4d ES X000, X008... VER TABILLA DE PINOS
     */
    else if (addr == 0xB000) {
        mapper.pattern1KBankNum[0] = (mapper.pattern1KBankNum[0] & 0xF0) | (balor & 0xF);
        mapper.patternTable1KOffset[0] = (mapper.pattern1KBankNum[0] & mapper.chrMask) * 1024;
    }
    else if (addr == 0xB001 || addr == 0xB004) {
        mapper.pattern1KBankNum[0] = (mapper.pattern1KBankNum[0] & 0xF) | ((balor & 0x1F)<<4);
        mapper.patternTable1KOffset[0] = (mapper.pattern1KBankNum[0] & mapper.chrMask) * 1024;
    }
    else if (addr == 0xB002 || addr == 0xB008) {
        mapper.pattern1KBankNum[1] = (mapper.pattern1KBankNum[1] & 0xF0) | (balor & 0xF);
        mapper.patternTable1KOffset[1] = (mapper.pattern1KBankNum[1] & mapper.chrMask) * 1024;
    }
    else if (addr == 0xB003 || addr == 0xB00C) {
        mapper.pattern1KBankNum[1] = (mapper.pattern1KBankNum[1] & 0xF) | ((balor & 0x1F)<<4);
        mapper.patternTable1KOffset[1] = (mapper.pattern1KBankNum[1] & mapper.chrMask) * 1024;
    }
    else if (addr == 0xC000) {
        mapper.pattern1KBankNum[2] = (mapper.pattern1KBankNum[2] & 0xF0) | (balor & 0xF);
        mapper.patternTable1KOffset[2] = (mapper.pattern1KBankNum[2] & mapper.chrMask) * 1024;
    }
    else if (addr == 0xC001 || addr == 0xC004) {
        mapper.pattern1KBankNum[2] = (mapper.pattern1KBankNum[2] & 0xF) | ((balor & 0x1F)<<4);
        mapper.patternTable1KOffset[2] = (mapper.pattern1KBankNum[2] & mapper.chrMask) * 1024;
    }
    else if (addr == 0xC002 || addr == 0xC008) {
        mapper.pattern1KBankNum[3] = (mapper.pattern1KBankNum[3] & 0xF0) | (balor & 0xF);
        mapper.patternTable1KOffset[3] = (mapper.pattern1KBankNum[3] & mapper.chrMask) * 1024;
    }
    else if (addr == 0xC003 || addr == 0xC00C) {
        mapper.pattern1KBankNum[3] = (mapper.pattern1KBankNum[3] & 0xF) | ((balor & 0x1F)<<4);
        mapper.patternTable1KOffset[3] = (mapper.pattern1KBankNum[3] & mapper.chrMask) * 1024;
    }
    else if (addr == 0xD000) {
        mapper.pattern1KBankNum[4] = (mapper.pattern1KBankNum[4] & 0xF0) | (balor & 0xF);
        mapper.patternTable1KOffset[4] = (mapper.pattern1KBankNum[4] & mapper.chrMask) * 1024;
    }
    else if (addr == 0xD001 || addr == 0xD004) {
        mapper.pattern1KBankNum[4] = (mapper.pattern1KBankNum[4] & 0xF) | ((balor & 0x1F)<<4);
        mapper.patternTable1KOffset[4] = (mapper.pattern1KBankNum[4] & mapper.chrMask) * 1024;
    }
    else if (addr == 0xD002 || addr == 0xD008) {
        mapper.pattern1KBankNum[5] = (mapper.pattern1KBankNum[5] & 0xF0) | (balor & 0xF);
        mapper.patternTable1KOffset[5] = (mapper.pattern1KBankNum[5] & mapper.chrMask) * 1024;
    }
    else if (addr == 0xD003 || addr == 0xD00C) {
        mapper.pattern1KBankNum[5] = (mapper.pattern1KBankNum[5] & 0xF) | ((balor & 0x1F)<<4);
        mapper.patternTable1KOffset[5] = (mapper.pattern1KBankNum[5] & mapper.chrMask) * 1024;
    }
    else if (addr == 0xE000) {
        mapper.pattern1KBankNum[6] = (mapper.pattern1KBankNum[6] & 0xF0) | (balor & 0xF);
        mapper.patternTable1KOffset[6] = (mapper.pattern1KBankNum[6] & mapper.chrMask) * 1024;
    }
    else if (addr == 0xE001 || addr == 0xE004) {
        mapper.pattern1KBankNum[6] = (mapper.pattern1KBankNum[6] & 0xF) | ((balor & 0x1F)<<4);
        mapper.patternTable1KOffset[6] = (mapper.pattern1KBankNum[6] & mapper.chrMask) * 1024;
    }
    else if (addr == 0xE002 || addr == 0xE008) {
        mapper.pattern1KBankNum[7] = (mapper.pattern1KBankNum[7] & 0xF0) | (balor & 0xF);
        mapper.patternTable1KOffset[7] = (mapper.pattern1KBankNum[7] & mapper.chrMask) * 1024;
    }
    else if (addr == 0xE003 || addr == 0xE00C) {
        mapper.pattern1KBankNum[7] = (mapper.pattern1KBankNum[7] & 0xF) | ((balor & 0x1F)<<4);
        mapper.patternTable1KOffset[7] = (mapper.pattern1KBankNum[7] & mapper.chrMask) * 1024;
    }
    else if (addr == 0xF000)
        mapper.irqReload = (mapper.irqReload & 0xF0) | (balor & 0xF);
    else if (addr == 0xF001 || addr == 0xF004)
        mapper.irqReload = (mapper.irqReload & 0xF) | ((balor & 0xF)<<4);
    else if (addr == 0xF002 || addr == 0xF008) {
        mapper.irqType = balor & 4;
        mapper.irqEnable = balor & 2;
        mapper.irqLoopFlag = balor & 1;
        if(mapper.irqEnable !=0) {
            mapper.irqCounter = mapper.irqReload;
            mapper.irqPreScaler +=  339;
        }
        if(ElMapper_GetInterruptSignal() == INTERRUPT_MAPPER_IRQ)
            LaCpu();
    }
    else if (addr == 0xF003 || addr == 0xF00C) {
        mapper.irqEnable = mapper.irqLoopFlag << 1;
        if(ElMapper_GetInterruptSignal() == INTERRUPT_MAPPER_IRQ)
            handleInterrupts();
    }
    else
        ElMapperBase_IoWrite(addr, balor);
}

int ElMapper023_IoRead(int addr) {
    if ((addr >= 0x6000) && (addr < 0x8000))
        return mapper.wRam[addr - 0x6000];
    else if ((addr >= 0x8000) && (addr < 0xA000))
        return ElCartujo_PrgRead(addr - 0x8000 + mapper.prg8KBank0Ptr);
    else if ((addr >= 0xA000) && (addr < 0xC000))
        return ElCartujo_PrgRead(addr - 0xA000 + mapper.prg8KBank1Ptr);
    else if ((addr >= 0xC000) && (addr < 0xE000))
        return ElCartujo_PrgRead(addr - 0xC000 + mapper.prg8KBank2Ptr);
    else if ((addr >= 0xE000) && (addr < 0x10000))
        return ElCartujo_PrgRead(addr - 0xE000 + mapper.prg8KBank3Ptr);
    else
        return ElMapperBase_IoRead(addr);
}

int ElMapper023_VramRead(int addr) {
    if(addr < 0x2000)
        return ElCartujo_ChrRead((addr%1024)+ mapper.patternTable1KOffset[((int)addr/0x400)]);
    else
        return ElMapperBase_VramRead(addr);
}

void ElMapper023_VramWrite(int addr, int elBalor) {
    if (addr < 0x2000) {
        if(ElCartujo_GetNumChr() == 0) // SOLO PUEDE ESCRIBIR EN CHR SI CHR RAM
            ElCartujo_ChrWrite((addr%1024)+ mapper.patternTable1KOffset[ ((int)addr/0x400)], elBalor);
    }
    else
        ElMapperBase_VramWrite(addr, elBalor);
}

void ElMapper023_Update(int losCiclos, int scanlineNum) {
    if(mapper.irqEnable !=0) {
        if(mapper.irqType == 0) {	// muedo scanline
            mapper.irqPreScaler-= losCiclos *3;
            if(mapper.irqPreScaler <=0) {
                mapper.irqCounter++;
                mapper.irqPreScaler += 339;
            }
        }
        else if(mapper.irqType == 4)	// muedo cycle
            mapper.irqCounter+= losCiclos;
        if(mapper.irqCounter > 0xff) {
            mapper.irqCounter = mapper.irqReload;
            ElMapper_SetInterruptSignal(INTERRUPT_MAPPER_IRQ);
        }
    }
}

void ElMapper023_Setup() {
    mapper.chrMask = (ElCartujo_GetNumChr() * 8) -1;
    mapper.prgMask = (ElCartujo_GetNumPrg() * 2) -1;
    mapper.prg8KBank0Ptr = 0;
    mapper.prg8KBank1Ptr = 8192;
    mapper.prg8KBank2Ptr = (ElCartujo_GetNumPrg() -1) * 16384;
    mapper.prg8KBank3Ptr = mapper.prg8KBank2Ptr + 8192;
    mapper.prgSwapBit = 0;
    mapper.irqPreScaler = 341;
    LaConsola_CargaMemoriaS32(&mapper.patternTable1KOffset, 8);
    LaConsola_CargaMemoriaS32(&mapper.pattern1KBankNum, 8);
    LaConsola_CargaMemoriaS32(&mapper.wRam, 0x2000);

    for(int n = 0; n< 8; n++)
        mapper.patternTable1KOffset[n] = n*1024;
    mapper.ioWrite = ElMapper023_IoWrite;
    mapper.ioRead = ElMapper023_IoRead;
    mapper.vramRead = ElMapper023_VramRead;
    mapper.vramWrite = ElMapper023_VramWrite;
    mapper.update = ElMapper023_Update;

}


void ElMapper_CargaAtributos() {
    switch(ElCartujo_GetMapperNum()) {
        case   0: ElMapperBase_Setup(); break;
    	case   1: ElMapper001_Setup(); break;
    	case   2: ElMapper002_Setup(); break;
    	case   4: ElMapper004_Setup(); break;

    	case   3: ElMapper003_Setup(); break;
    //	case   7: mapper = new Mapper007(this); break;
        case  23: ElMapper023_Setup(); break;

    //	case  71: mapper = new Mapper071(this); break;

        /*		case   4: mapper = new Mapper004(this); break;
        case   5: mapper = new Mapper005(this); break;


        case   9: mapper = new Mapper009(this); break;
        case  10: mapper = new Mapper010(this); break;

        case  16: mapper = new Mapper016(this); break;

        case  18: mapper = new Mapper018(this); break;
        case  19: mapper = new Mapper019(this); break;

        case  21: mapper = new Mapper021(this); break;
        case  22: mapper = new Mapper022(this); break;
        case  23: mapper = new Mapper023(this); break;

        case  25: mapper = new Mapper025(this); break;
        case  26: mapper = new Mapper026(this); break;

        case  32: mapper = new Mapper032(this); break;
        case  33: mapper = new Mapper033(this); break;
        case  34: mapper = new Mapper034(this); break;

        case  65: mapper = new Mapper065(this); break;
        //case  66: mapper = new Mapper066(this); break;
        case  67: mapper = new Mapper067(this); break;
        case  68: mapper = new Mapper068(this); break;

        case  70: mapper = new Mapper070(this); break;

        case  72: mapper = new Mapper072(this); break;

        case  75: mapper = new Mapper075(this); break;

        case  78: mapper = new Mapper078(this); break;
        case  79: mapper = new Mapper079(this); break;
        case  80: mapper = new Mapper080(this); break;

        case  87: mapper = new Mapper087(this); break;
        case  88: mapper = new Mapper088(this); break;

        case  89: mapper = new Mapper089(this); break;

        case  91: mapper = new Mapper091(this); break;

        case  93: mapper = new Mapper093(this); break;
        case  94: mapper = new Mapper094(this); break;
        case  95: mapper = new Mapper095(this); break;

        case  97: mapper = new Mapper097(this); break;

        case 118: mapper = new Mapper118(this); break;
        case 119: mapper = new Mapper119(this); break;

        case 140: mapper = new Mapper140(this); break;

        case 152: mapper = new Mapper070(this); break;

        case 154: mapper = new Mapper154(this); break;

        case 180: mapper = new Mapper180(this); break;

        case 184: mapper = new Mapper184(this); break;

        case 206: mapper = new Mapper206(this); break;
        case 210: mapper = new Mapper210(this); break;
    */
    }
}
