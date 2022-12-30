#include "LaConsola.h"

LaPpu ppu;


bool LaPpu_Carga() {
    bool elSuceso = true;
    ppu.baseSprPatternIndex = BASE_SPR_TABLE;
    ppu.sprHeight = SMALL_SPRITE;
    if (
        LaConsola_CargaMemoriaS32(&ppu.sprOam, 256) &
        LaConsola_CargaMemoriaS32(&ppu.drawBuffer, DRAWBUFFER_WIDTH * DRAWBUFFER_HEIGHT) &
        LaConsola_CargaMemoriaS32(&ppu.palettes, 28)
    ) {}
    else {
        printf("error cargando PPU, lanza tu computadora en Rio Ypacarai !!!\n");
        elSuceso = false;
    }
    memset(ppu.palettes, 0, sizeof(int32_t) * 28);
    return elSuceso;
}

void LaPpu_Descarga() {
    free(ppu.sprOam);
    free(ppu.drawBuffer);
    free(ppu.palettes);
}

void LaPpu_TurnOnFlags(uint8_t losFlagos) {
    ppu.flagsRegister |= losFlagos;
}

void LaPpu_TurnOffFlags(uint8_t losFlagos) {
    ppu.flagsRegister = (ppu.flagsRegister & (~(losFlagos))) & 0xff;
}

void LaPpu_ResetFlags() {
    LaPpu_TurnOffFlags(NMI_FLAG | SPR0HIT_FLAG | SPROVERFLOW_FLAG);
}

bool LaPpu_FlagIsSet(uint8_t elFlago) {

    if((ppu.flagsRegister & elFlago) != 0)
        return true;
    else
        return false;

}

uint8_t LaPpu_GetFlagsRegister() {
    return ppu.flagsRegister;
}

uint8_t LaPpu_SprOamRead(uint8_t addr) {
    return ppu.sprOam[addr];
}

void LaPpu_SprOamWrite(uint8_t addr, uint8_t val) {
    ppu.sprOam[addr] = val;
}

int32_t LaPpu_PalettesRead(int32_t addr) {
    return ppu.palettes[addr];
}

void LaPpu_PalettesWrite(uint8_t addr, uint8_t val) {
    ppu.palettes[addr] = val;
}

uint8_t getSprZeroBit(uint8_t spriteNumber) {

    if(spriteNumber == 0)
        return SPR0_HIT_BIT_ON;
    else
        return SPR0_HIT_BIT_OFF;

}

uint8_t getSprPriorityBit(uint8_t spriteNumber) {

    if((ppu.sprOam[spriteNumber + 2] & SPR_PRIORITY_BIT_MASK) !=0)
        return SPR_PRIORITY_BIT_ON;
    else
        return SPR_PRIORITY_BIT_OFF;

}

bool getSprFlipYBit(uint8_t spriteNumber) {

    if((ppu.sprOam[spriteNumber + 2] & SPR_FLIPY_BIT_MASK) !=0)
        return true;
    else
        return false;

}

bool getSprFlipXBit(uint8_t spriteNumber) {

    if((ppu.sprOam[spriteNumber + 2] & SPR_FLIPX_BIT_MASK) !=0)
        return true;
    else
        return false;

}

uint8_t getSprPixelsToWrite(uint8_t spriteNumber) {

    if(ppu.sprOam[spriteNumber + 3] > SPR_MAX_COORDX)
        return 0x100 - ppu.sprOam[spriteNumber + 3];
    else
        return SPR_WIDTH;

}

uint8_t getSprHeight() {
    return ppu.sprHeight;
}

void LaPpu_SetFineX(uint16_t balor) {
    ppu.fineX = balor;
}

void LaPpu_SetLoopyV(uint16_t balor) {
    ppu.loopyV = balor;
}

void LaPpu_SetLoopyT(uint16_t balor) {
    ppu.loopyT = balor;
}

uint16_t LaPpu_GetLoopyV() {
    return ppu.loopyV;
}

uint16_t LaPpu_GetLoopyT() {
    return ppu.loopyT;
}

bool LaPpu_IsRenderActive() {

    if (ppu.sprRenderEnabled || ppu.bgRenderEnabled)
        return true;
    else
        return false;

}

void LaPpu_RenderSprites(uint16_t currScanline) {
    for(int16_t spriteIndex = LAST_SPRITE_Y; spriteIndex > -1; spriteIndex-= SPRITE_ENTRY_SIZE) {

        ppu.sprFirstLine = ppu.sprOam[spriteIndex];

        if (currScanline >= ppu.sprFirstLine && currScanline < (ppu.sprFirstLine + ppu.sprHeight)) {
            ppu.spriteCount++;
            if(ppu.spriteCount >= MAX_SPRITE_COUNT)
                LaPpu_TurnOnFlags(SPROVERFLOW_FLAG);

            ppu.sprZeroBit = getSprZeroBit(spriteIndex);
            ppu.sprPriorityBit = getSprPriorityBit(spriteIndex);

            if (ppu.sprHeight == BIG_SPRITE)
                ppu.sprTileNum = ((ppu.sprOam[spriteIndex + 1] & 1) * BASE_SPR_TABLE) | (ppu.sprOam[spriteIndex + 1] & 0xfe);
            else
                ppu.sprTileNum = ppu.baseSprPatternIndex + ppu.sprOam[spriteIndex + 1];

            if(getSprFlipYBit(spriteIndex) == true)
                ppu.sprTileLinOffset = (ppu.sprHeight - 1) - (currScanline - ppu.sprFirstLine);
            else
                ppu.sprTileLinOffset = (currScanline - ppu.sprFirstLine);

            if (ppu.sprTileLinOffset > 7)
                ppu.sprTileLinOffset += 8;

            ppu.sprTileAddr = (16 * ppu.sprTileNum) + ppu.sprTileLinOffset;

            ppu.vramReadCaller = VRAM_READ_CALLER_SPR;

            ppu.sprPatternByte1 = mapper.vramRead(ppu.sprTileAddr);
            ppu.sprPatternByte2 = mapper.vramRead(ppu.sprTileAddr + 8);

            ppu.sprPixelsToWrite = getSprPixelsToWrite(spriteIndex);

            ppu.renderBufferCursor = (currScanline * DRAWBUFFER_WIDTH) + ppu.sprOam[spriteIndex + 3];

            if (getSprFlipXBit(spriteIndex)) {	// Si Flip X
                ppu.sprDirection = 1;
                ppu.sprTileColOffset = 0;
            }
            else {
                ppu.sprDirection = -1;
                ppu.sprTileColOffset = 7;
            }

            for (uint16_t pixelIndex = 0; pixelIndex < ppu.sprPixelsToWrite; pixelIndex++) {
                ppu.sprPatternFinal = ((ppu.sprPatternByte1 >> ppu.sprTileColOffset) & 1) | (((ppu.sprPatternByte2 >> ppu.sprTileColOffset) & 1) << 1);

                if ((ppu.sprPatternFinal & PIXEL_OPAQUE_MASK) == 0) // si SPR PATTERN transparente entonces NO DESENIA
                    ppu.renderBufferCursor++;
                // si spr pixel < 8 y left col disabled NO DESENIA
                else if ((ppu.sprOam[spriteIndex + 3] + pixelIndex) < 8 && ppu.sprLeftColumnEnabled == false)
                    ppu.renderBufferCursor++;
                else 	// de octra forma desenia el Spritz
                {
                    if((ppu.renderBufferCursor+1) % 256 == 0) {
                //		System.out.printf("aca era %x ahora zero\n", sprZeroBit);
                        ppu.sprZeroBit = 0;
                    }
                    ppu.drawBuffer[ppu.renderBufferCursor++] = SPR_COLLISION_BIT | ppu.sprPatternFinal |
                        ((ppu.sprOam[spriteIndex + 2] & PIXEL_OPAQUE_MASK) << 2) | ppu.sprZeroBit | ppu.sprPriorityBit;
                }

                ppu.sprTileColOffset += ppu.sprDirection;
            }
        }
    }

}

void LaPpu_RenderScanline(uint16_t ciclosEjecutar, uint16_t scanlineNum) {
    uint8_t bgPattern;

    for (uint16_t pixelIndex = 1; pixelIndex <= ciclosEjecutar; pixelIndex++) {

        if(--ppu.spr0FlagDelay == 0)
            LaPpu_TurnOnFlags(SPR0HIT_FLAG);

        if(ppu.lineDot == 1 && scanlineNum == 241) {
            LaPpu_TurnOnFlags(NMI_FLAG);

            if (ElMapper_GetNmiEnableFlag()) {
                if(ppu.suprimeNmi == false)
                    ElMapper_SetInterruptSignal(INTERRUPT_NMI);
            }

            LaPpu_SetSuprimeNmiFlag(false);
        }

        if(ppu.lineDot == 1 && scanlineNum == 261) {
            LaPpu_ResetFlags();
            ppu.sprHitFrameFlag = false;
        }

        if(ppu.lineDot == 256) {

            if(ppu.fue255)
                ppu.fue255 = false;
            else {
                if (LaPpu_IsRenderActive() && (scanlineNum < 240))
                    LaPpu_UpdateCoarseY();
            }

        }

        if(ppu.lineDot == 257) {

            if(ppu.fue255)
                ppu.fue255 = false;
            else {
                if(LaPpu_IsRenderActive() && (scanlineNum < 240))
                 ppu.loopyV = (ppu.loopyV & 0x7BE0) | (ppu.loopyT & 0x41F);
                    ElMapper_SetReg2003(0);

            }

        }

        if(ppu.lineDot == 304) {

            if (LaPpu_IsRenderActive() && (scanlineNum == 261))
                ppu.loopyV = (ppu.loopyV & 0x841f) | (ppu.loopyT & 0x7BE0);

        }

        if(ppu.lineDot == 340 && scanlineNum == 261) {

            if (ppu.bgRenderEnabled)
                LaPpu_UpdateCoarseY();

            }

        if (ppu.bgColunaDrawCursor < DRAWBUFFER_WIDTH) {

            if(ppu.bgRenderEnabled) {
                bgPattern = ((ppu.bgPatternByte1 >> ppu.bgTileColOffset) & 1) | (((ppu.bgPatternByte2 >> ppu.bgTileColOffset) & 1) << 1);

                if (bgPattern == 0 || (ppu.bgColunaDrawCursor < 8 && ppu.bgLeftColumnEnabled == false))
                    ppu.renderBufferCursor++;

                else {

                    if ((ppu.drawBuffer[ppu.renderBufferCursor] & PIXEL_OPAQUE_MASK) != 0) {

                        if ((ppu.drawBuffer[ppu.renderBufferCursor] & SPR_PRIORITY_BIT_MASK) !=0) {

                            if (ppu.sprHitFrameFlag == false) {
                                ppu.spr0FlagDelay = 3;
                                ppu.sprHitFrameFlag = true;
                            }

                        }

                        if ((ppu.drawBuffer[ppu.renderBufferCursor] & SPR_PRIORITY_BIT_ON) !=0)
                            ppu.drawBuffer[ppu.renderBufferCursor] = bgPattern |
                                (((ppu.attrByte >> ppu.attrShift) & PIXEL_OPAQUE_MASK) << 2);

                    }
                    else
                        ppu.drawBuffer[ppu.renderBufferCursor] = bgPattern |
                            (((ppu.attrByte >> ppu.attrShift) & PIXEL_OPAQUE_MASK) << 2);

                   ppu.renderBufferCursor++;

                }

                ppu.bgTileColOffset--;

                if (ppu.bgTileColOffset < 0) {
                    LaPpu_UpdateCoarseX();
                    ppu.vramReadCaller = VRAM_READ_CALLER_BG;
                    ppu.bgTile = mapper.vramRead(BASE_NAMETABLE_ADDR | (ppu.loopyV & 0xFFF));
                    ppu.bgTileAddr = ppu.baseBgPatternIndex + 16 * ppu.bgTile + ppu.fineY;
                    ppu.bgPatternByte1 = mapper.vramRead(ppu.bgTileAddr);
                    ppu.bgPatternByte2 = mapper.vramRead(ppu.bgTileAddr + 8);
                    ppu.bgTileColOffset = 7;
                    ppu.attrByte = mapper.vramRead(0x23C0 | (ppu.loopyV & 0x0C00) |
                        ((ppu.loopyV >> 4) & 0x38) | ((ppu.loopyV >> 2) & 0x07));
                    ppu.attrShift = (((ppu.loopyV & 2) >> 1) | ((ppu.loopyV & 0x40) >> 5)) * 2;

                }
            }
        }

        ppu.bgColunaDrawCursor++;
        ppu.lineDot++;

        if(ppu.lineDot > 340) {
            ppu.ciclosSobrando = ciclosEjecutar - pixelIndex;
            ppu.lineDot-=341;
            return;
        }

        ppu.ciclosSobrando = ciclosEjecutar - pixelIndex;
    }
}

void LaPpu_UpdateBgLine(int currScanline) {
    ppu.renderBufferCursor = (currScanline * DRAWBUFFER_WIDTH);
    ppu.vramReadCaller = VRAM_READ_CALLER_BG;
    ppu.bgTile = mapper.vramRead(BASE_NAMETABLE_ADDR | (ppu.loopyV & 0xFFF));
    ppu.fineY = (ppu.loopyV & 0x7000) >> 12;
    ppu.bgTileAddr = ppu.baseBgPatternIndex + 16 * ppu.bgTile + ppu.fineY;
    ppu.bgPatternByte1 = mapper.vramRead(ppu.bgTileAddr);
    ppu.bgPatternByte2 = mapper.vramRead(ppu.bgTileAddr + 8);
    ppu.bgTileColOffset = ppu.fineX == 0 ? 7 : 7 - ppu.fineX;
    ppu.attrByte = mapper.vramRead(0x23C0 | (ppu.loopyV & 0x0C00) |
        ((ppu.loopyV >> 4) & 0x38) | ((ppu.loopyV >> 2) & 0x07));
    ppu.attrShift = (((ppu.loopyV & 2) >> 1) | ((ppu.loopyV & 0x40) >> 5)) * 2;
    ppu.bgColunaDrawCursor= 0;
}

void LaPpu_UpdateCoarseX() {

    if ((ppu.loopyV & 0x1F) == 0x1f) {
        ppu.loopyV &= ~0x001F;
        ppu.loopyV ^= 0x400;
    }
    else
        ppu.loopyV++;

}


void LaPpu_UpdateCoarseY() {

    if ((ppu.loopyV & 0x7000) != 0x7000) // si Fine Y < 7
        ppu.loopyV += 0x1000;             // incremienta Fine Y
    else {                                   // si 7 hay que 1) resetar Fine y 2) incrementar Coarse Y
            ppu.loopyV &= ~0x7000;             // si Fine Y == 7, hay que resetar para 0
        int coarseY = (ppu.loopyV & 0x3E0) >> 5;

        if (coarseY == 29) {                  // si Linea == 29 hay que saltar para proxima name table
            coarseY = 0;
//    		if(isRenderActive())
            ppu.loopyV ^= 0x800;           // aca pula las lineas 30 y 31 (attr tables y salta para proxima nt)
        }
        else if (coarseY == 31)              // si fue para 31 y no pulo attr tables es en la verdade 0
            coarseY = 0;
        else                                // de octra forma solamente incremienta coarseY
            coarseY++;
        ppu.loopyV = (ppu.loopyV & ~0x03e0) | (coarseY << 5);
   }

}

void LaPpu_Reg2000Write(int value) {
    ppu.loopyT = (ppu.loopyT & 0x73ff) | ((value & 3) << 10);

    if((value & 0x20) == 0x20)
        ppu.sprHeight = BIG_SPRITE;
    else
        ppu.sprHeight = SMALL_SPRITE;

    if((value & 0x10) == 0x10)
        ppu.baseBgPatternIndex = 0x1000;
    else
        ppu.baseBgPatternIndex = 0;

    if((value & 8) == 8)
        ppu.baseSprPatternIndex =  256;
    else
        ppu.baseSprPatternIndex =  0;

}


void LaPpu_Reg2001Write(int value) {
    ppu.sprRenderEnabled = (value & 0x10) == 0x10;
    ppu.bgRenderEnabled =  (value & 8) == 8;
    ppu.sprLeftColumnEnabled = (value & 4) == 4;
    ppu.bgLeftColumnEnabled = (value & 2) == 2;
}

void LaPpu_ResetRenderBuffer() {
    memset(ppu.drawBuffer, 0, sizeof(uint32_t) * DRAWBUFFER_WIDTH * DRAWBUFFER_HEIGHT);
    ppu.renderBufferCursor = 0;
}

int32_t LaPpu_GetLineDot() {
    return ppu.lineDot;
}

void LaPpu_SetSuprimeNmiFlag(bool balor) {
    ppu.suprimeNmi = balor;
}

void LaPpu_SetVramCaller(int balor) {
    ppu.vramReadCaller = balor;
}

