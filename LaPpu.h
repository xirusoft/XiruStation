#ifndef _FAMILY_GAME
#define _FAMILY_GAME

#include "ElMapper.h"

#define NMI_FLAG 0x80
#define SPR0HIT_FLAG 0x40
#define SPROVERFLOW_FLAG 0x20
#define BASE_SPR_TABLE 256
#define SMALL_SPRITE 8
#define BIG_SPRITE 16
#define DRAWBUFFER_WIDTH 256
#define DRAWBUFFER_HEIGHT 240
#define SPR0_HIT_BIT_ON 0x20
#define SPR0_HIT_BIT_OFF 0
#define SPR_PRIORITY_BIT_MASK 0x20
#define SPR_PRIORITY_BIT_ON 0x40
#define SPR_PRIORITY_BIT_OFF 0
#define SPR_FLIPY_BIT_MASK 0x80
#define SPR_FLIPX_BIT_MASK 0x40
#define SPR_WIDTH 8
#define SPR_MAX_COORDX 248
#define LAST_SPRITE_Y 252
#define SPRITE_ENTRY_SIZE 4
#define MAX_SPRITE_COUNT 8
#define VRAM_READ_CALLER_SPR 1
#define VRAM_READ_CALLER_BG 0
#define PIXEL_OPAQUE_MASK 3
#define SPR_COLLISION_BIT 0x10
#define BASE_NAMETABLE_ADDR 0x2000


struct LaPpu
{
    int32_t flagsRegister;
    int32_t baseBgPatternIndex;                // EL BASE PATTERN TABLE (0 O 0x1000 DE ACUERDO CON PPU REGISTERS)
    int32_t baseSprPatternIndex;               // EL BASE SPR PATTERN (0 O 0x1000)
    int32_t sprHeight;                     // SPR 8 O 16 PIXELS ALTO (SET EN PPU REGISTER)
    int32_t *sprOam;                 // SPR OAM
    int32_t loopyT;
    int32_t loopyV;
    int32_t fineY;
    int32_t fineX;
    bool bgRenderEnabled;               // Enable Render (SET EN PPU REGISTERS)
    bool sprRenderEnabled;
    bool bgLeftColumnEnabled;           // Left 8 pixels Enable flags
    bool sprLeftColumnEnabled;
    int32_t bgPatternByte1;
    int32_t bgPatternByte2;
    int32_t attrByte;
    int32_t attrShift;
    int32_t bgTile;
    int32_t bgTileAddr;
    int32_t bgTileColOffset;
    int32_t sprPatternByte1;
    int32_t sprPatternByte2;
    int32_t sprPatternFinal;
    int32_t sprFirstLine;
    int32_t sprTileNum;
    int32_t sprTileAddr;
    int32_t sprDirection;
    int32_t sprTileColOffset;
    int32_t sprTileLinOffset;
    int32_t spriteCount;
    int32_t sprPixelsToWrite;
    int32_t sprZeroBit;                   // FLAG PARA MARCAR SI EL SPRITZ ES ZERO (PARA CALCULO DE HIT)
    int32_t sprPriorityBit;              // ATTRIBUTE DE SPRITE, SE AL FRENTE O ACTRAS DE BACKGROUND
    bool sprHitFrameFlag;              // ex yafuesprhit EL SPR0 HIT SOLO OCURRE UNA VEZ POR FRAME
    int32_t *palettes;               // ex paletadatos LAS ENTRIES REALES DE PALETAS
    int32_t lineDot;
    int32_t ciclosSobrando;
    int32_t spr0FlagDelay;
    int32_t vramReadCaller;
    bool fue255;
    bool suprimeNmi;

    int32_t bgColunaDrawCursor;
    int32_t renderBufferCursor;           // LOS CURSORES PARA DESENIAR PROGRESIVAMENTE
    int32_t *drawBuffer;      // EL BUFFER QUE SE DESENIA


};
extern LaPpu ppu;

bool LaPpu_Carga();
void LaPpu_Descarga();
void LaPpu_TurnOnFlags(uint8_t);
void LaPpu_TurnOffFlags(uint8_t);
void LaPpu_ResetFlags();
uint8_t LaPpu_GetFlagsRegister();
uint8_t LaPpu_SprOamRead(uint8_t);
void LaPpu_SprOamWrite(uint8_t, uint8_t );
int32_t LaPpu_PalettesRead(int32_t);
void LaPpu_PalettesWrite(uint8_t, uint8_t);
void LaPpu_SetFineX(uint16_t);
void LaPpu_SetLoopyV(uint16_t);
void LaPpu_SetLoopyT(uint16_t);
uint16_t LaPpu_GetLoopyV();
uint16_t LaPpu_GetLoopyT();
void LaPpu_RenderSprites(uint16_t);
void LaPpu_RenderScanline(uint16_t, uint16_t);
int32_t LaPpu_GetLineDot();
void LaPpu_SetSuprimeNmiFlag(bool);
bool LaPpu_IsRenderActive();
void LaPpu_UpdateCoarseY();
void LaPpu_UpdateCoarseX();
void LaPpu_UpdateBgLine(int);
void LaPpu_Reg2000Write(int);
void LaPpu_Reg2001Write(int);
void LaPpu_ResetRenderBuffer();
bool LaPpu_FlagIsSet(uint8_t);
void LaPpu_SetVramCaller(int);
#endif
