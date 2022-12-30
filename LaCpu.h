#ifndef _TURBOGAME
#define _TURBOGAME

#include "ElMapper.h"
#include "LaPpu.h"

#define CARRY_FLAG 1
#define ZERO_FLAG 2
#define INTERRUPT_FLAG 4
#define DECIMAL_FLAG 8
#define BREAK_FLAG 0x10
#define BIT_5 0x20
#define OVERFLOW_FLAG 0x40
#define NEGATIVE_FLAG 0x80

#include <stdio.h>
#include <stdlib.h>

struct LaCpu {
    uint8_t regX;
    uint8_t regY;
    uint8_t regA;
    uint8_t regSP;
    uint8_t flagsRegister;
    uint16_t currentCycles;
    uint16_t PC;
    uint16_t currentOpcode;
    uint8_t *opcodeCycles;
    uint8_t deltaCycles;
    uint8_t currentOpCycles;
    bool updateInterruptCycles;

    uint32_t totalInstruc;
    uint32_t totalCiclos;
};

extern LaCpu cpu;

bool LaCpu_Carga();
void LaCpu_Descarga();
uint16_t LaCpu_GetCurrentCycles();
void LaCpu_UpdateCurrentCycles(uint16_t);
uint16_t LaCpu_ExecuteInstruction();
uint16_t LaCpu_GetCurrentOpCycles();

#endif
