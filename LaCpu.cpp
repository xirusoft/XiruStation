#include "LaConsola.h"

LaCpu cpu;

bool LaCpu_Carga() {

    bool elSuceso = true;

    cpu.opcodeCycles = (uint8_t*)malloc(sizeof(uint8_t) * 256);

    if(cpu.opcodeCycles == NULL) {
        printf("error cargando CPU, cubre tu computadora con mierda de cerdos !!!\n");
        elSuceso = false;
    }

    cpu.regSP = 0xff;
    cpu.flagsRegister = BREAK_FLAG;
    cpu.PC = ElMapper_GetRstAddr();
    cpu.currentOpcode = mapper.ioRead(cpu.PC);
    uint8_t ciclios[] = {
        		7, 6, 2, 7, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
                2, 5, 2, 7, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 6, 7,
			    6, 6, 2, 7, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
			    2, 5, 2, 7, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
			    6, 6, 2, 7, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
			    2, 5, 2, 7, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
			    6, 6, 2, 7, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
			    2, 5, 2, 7, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
			    2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
			    2, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
                2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
                2, 5, 2, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
                2, 6, 2, 7, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
                2, 5, 2, 7, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
                2, 6, 3, 7, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
                2, 5, 2, 7, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7
    };
    for (uint n = 0; n < 256; n++)
        cpu.opcodeCycles[n] = ciclios[n];
    return elSuceso;
}

void LaCpu_Descarga() {
    free(cpu.opcodeCycles);
}

uint8_t getValImmed() {
    return mapper.ioRead(cpu.PC + 1);
}

uint32_t getAddr_Idx_Ind() {
    uint16_t addrLsb = (getValImmed() + cpu.regX) & 0xFF;
    return mapper.ioRead(addrLsb) +
        (mapper.ioRead(((addrLsb + 1) & 0xFF)) << 8);
}

uint32_t getAddr_Ind() {
    return mapper.ioRead(getValImmed()) + (mapper.ioRead((getValImmed() + 1) & 0xFF) << 8);
}

uint32_t getAddr_Ind_Idx() {
    return (getAddr_Ind() + cpu.regY) & 0xFFFF;
}

uint32_t getAddr_Abs() {
    return mapper.ioRead(cpu.PC + 1) | (mapper.ioRead(cpu.PC + 2) << 8);
}

void turnOnFlags(uint8_t losflagsRegister) {
    cpu.flagsRegister |= losflagsRegister;
}

void turnOffFlags(uint8_t losflagsRegister) {
    cpu.flagsRegister = cpu.flagsRegister & (~losflagsRegister) & 0xff;
}

bool flagIsSet(uint8_t elFlago) {

    if((cpu.flagsRegister & elFlago) != 0)
        return true;
    else
        return false;

}

void pushByte(uint8_t elBalor) {
    mapper.ioWrite(0x100 + cpu.regSP, elBalor);
    cpu.regSP = (cpu.regSP - 1) & 0xff;
}

uint8_t pullByte(uint8_t numOpcodes) {
    cpu.PC+= numOpcodes;
    cpu.regSP = (cpu.regSP + 1) & 0xff;
    return mapper.ioRead(0x100 + cpu.regSP);
}

void setRegA(uint8_t elBalor) {
    cpu.regA = elBalor;

    if ((cpu.regA & 0x80) == 0x80)
        turnOnFlags(NEGATIVE_FLAG);
    else
        turnOffFlags(NEGATIVE_FLAG);
    if (cpu.regA == 0)
        turnOnFlags(ZERO_FLAG);
    else
        turnOffFlags(ZERO_FLAG);

}

void setRegY(uint8_t elBalor) {
    cpu.regY = elBalor;

    if ((cpu.regY & 0x80) == 0x80)
        turnOnFlags(NEGATIVE_FLAG);
    else
        turnOffFlags(NEGATIVE_FLAG);

    if (cpu.regY == 0)
        turnOnFlags(ZERO_FLAG);
    else
        turnOffFlags(ZERO_FLAG);

}

void setRegX(uint8_t elBalor) {
    cpu.regX = elBalor;

    if ((cpu.regX & 0x80) == 0x80)
        turnOnFlags(NEGATIVE_FLAG);
    else
        turnOffFlags(NEGATIVE_FLAG);

    if (cpu.regX == 0)
        turnOnFlags(ZERO_FLAG);
    else
        turnOffFlags(ZERO_FLAG);

}

void checkBugIndIdx() {
    uint32_t temp = getAddr_Ind();

    if (((temp ^ (temp + cpu.regY)) & 0x100) == 0x100)
        cpu.currentCycles++;

}

void checkBugAbsInd(uint8_t rego) {
    uint32_t temp = getAddr_Abs();

    if (((temp ^ (temp + rego)) & 0x100) == 0x100)
        cpu.currentCycles++;

}

void opOr(uint8_t elOperando) {
    cpu.regA |= elOperando;

    if ((cpu.regA & 0x80) == 0x80)
        turnOnFlags(NEGATIVE_FLAG);
    else
        turnOffFlags(NEGATIVE_FLAG);

    if (cpu.regA == 0)
        turnOnFlags(ZERO_FLAG);
    else
        turnOffFlags(ZERO_FLAG);

}

void opAnd(uint8_t elOperando) {
    cpu.regA &= elOperando;

    if ((cpu.regA & 0x80) == 0x80)
        turnOnFlags(NEGATIVE_FLAG);
    else
        turnOffFlags(NEGATIVE_FLAG);

    if (cpu.regA == 0)
        turnOnFlags(ZERO_FLAG);
    else
        turnOffFlags(ZERO_FLAG);

}

void opAndXY(uint8_t elReg, uint8_t elOffset) {		// El Illegal Opcode 9C op And Y con ((opcode+2)+1) y store en (Abs,X)

    uint8_t temp = (mapper.ioRead(cpu.PC + 2)+1) & elReg;

    if ((temp & 0x80) == 0x80)
        turnOnFlags(NEGATIVE_FLAG);
    else
        turnOffFlags(NEGATIVE_FLAG);

    if (temp == 0)
        turnOnFlags(ZERO_FLAG);
    else
        turnOffFlags(ZERO_FLAG);

    mapper.ioWrite((getAddr_Abs() + elOffset) & 0xffff, temp);

}

void opEor(uint8_t elOperando) {

    cpu.regA ^= elOperando;

    if ((cpu.regA & 0x80) == 0x80)
        turnOnFlags(NEGATIVE_FLAG);
    else
        turnOffFlags(NEGATIVE_FLAG);

    if (cpu.regA == 0)
        turnOnFlags(ZERO_FLAG);
    else
        turnOffFlags(ZERO_FLAG);

}

void opBit(uint8_t elOperando) {
    turnOffFlags(NEGATIVE_FLAG | OVERFLOW_FLAG | ZERO_FLAG);
    cpu.flagsRegister |= elOperando & 0xC0;

    if ((cpu.regA & elOperando) == 0)
        turnOnFlags(ZERO_FLAG);

}

uint8_t opAsl(uint8_t elOperando) {

    if ((elOperando & 0x80) == 0x80)
        turnOnFlags(CARRY_FLAG);
    else
        turnOffFlags(CARRY_FLAG);

    elOperando = (elOperando << 1) & 0xFF;

    if ((elOperando & 0x80) == 0x80)
        turnOnFlags(NEGATIVE_FLAG);
    else
        turnOffFlags(NEGATIVE_FLAG);

    if (elOperando == 0)
        turnOnFlags(ZERO_FLAG);
    else
        turnOffFlags(ZERO_FLAG);

    return elOperando;
}

uint8_t opRol(uint8_t elOperando) {
    uint8_t tempor = (elOperando & 0x80) == 0x80 ? 1 : 0;
    elOperando = ((elOperando << 1) & 0xFF) | (cpu.flagsRegister & CARRY_FLAG);
    turnOffFlags(NEGATIVE_FLAG | ZERO_FLAG | CARRY_FLAG);

    if ((elOperando & 0x80) == 0x80)
        turnOnFlags(NEGATIVE_FLAG);

    if (elOperando == 0)
        turnOnFlags(ZERO_FLAG);

    cpu.flagsRegister |= tempor;
    return elOperando;
}

uint8_t opRor(uint8_t elOperando) {
    uint8_t tempor = (elOperando & 1) == 1 ? 1 : 0;
    elOperando = (elOperando >> 1) | ((cpu.flagsRegister & CARRY_FLAG) << 7);
    turnOffFlags(NEGATIVE_FLAG | ZERO_FLAG | CARRY_FLAG);

    if ((elOperando & 0x80) == 0x80)
        turnOnFlags(NEGATIVE_FLAG);

    if (elOperando == 0)
        turnOnFlags(ZERO_FLAG);

    cpu.flagsRegister |= tempor;
    return elOperando;
}

void opCmp(uint8_t elBalor) {
    uint8_t laDiferencia = cpu.regA - elBalor;

    if (cpu.regA >= elBalor)
        turnOnFlags(CARRY_FLAG);
    else
        turnOffFlags(CARRY_FLAG);

    if ((laDiferencia & 0x80) == 0x80)
        turnOnFlags(NEGATIVE_FLAG);
    else
        turnOffFlags(NEGATIVE_FLAG);

    if (laDiferencia == 0)
        turnOnFlags(ZERO_FLAG);
    else
        turnOffFlags(ZERO_FLAG);

}

void opCpx(uint8_t elBalor) {
    uint8_t laDiferencia = cpu.regX - elBalor;

    if (cpu.regX >= elBalor)
        turnOnFlags(CARRY_FLAG);
    else
        turnOffFlags(CARRY_FLAG);

    if ((laDiferencia & 0x80) == 0x80)
        turnOnFlags(NEGATIVE_FLAG);
    else
        turnOffFlags(NEGATIVE_FLAG);

    if (laDiferencia == 0)
        turnOnFlags(ZERO_FLAG);
    else
        turnOffFlags(ZERO_FLAG);
}

void opCpy(uint8_t elBalor) {
    uint8_t laDiferencia = cpu.regY - elBalor;

    if (cpu.regY >= elBalor)
        turnOnFlags(CARRY_FLAG);
    else
        turnOffFlags(CARRY_FLAG);

    if ((laDiferencia & 0x80) == 0x80)
        turnOnFlags(NEGATIVE_FLAG);
    else
        turnOffFlags(NEGATIVE_FLAG);

    if (laDiferencia == 0) {
        turnOnFlags(ZERO_FLAG);
    }
    else
        turnOffFlags(ZERO_FLAG);

}

void opAdc(uint8_t elOperando) {
    uint32_t tempor = elOperando + cpu.regA + (cpu.flagsRegister & CARRY_FLAG);
    turnOffFlags(NEGATIVE_FLAG | OVERFLOW_FLAG | ZERO_FLAG | CARRY_FLAG);

    if ((((cpu.regA ^ elOperando) & 0x80) == 0) && (((cpu.regA ^ tempor) & 0x80) != 0))
        turnOnFlags(OVERFLOW_FLAG);

    if ((tempor & 0x80) == 0x80)
        turnOnFlags(NEGATIVE_FLAG);

    if ((tempor & 0xFF) == 0)
        turnOnFlags(ZERO_FLAG);

    if (tempor > 0xFF)
        turnOnFlags(CARRY_FLAG);

    cpu.regA = tempor & 0xFF;
}

void opSbc(uint8_t elOperandu) { // SBC: Stack Overflow "solo inverter los bits del operando y opr ADC normal", dijo el hijo de puta
    elOperandu = (~elOperandu) & 0xFF;
    opAdc(elOperandu);
}

uint8_t opIncDec(uint8_t elOperando, int8_t elSinal) {
    elOperando = (elOperando + elSinal) & 0xff;

    if ((elOperando & 0x80) == 0x80)
        turnOnFlags(NEGATIVE_FLAG);
    else
        turnOffFlags(NEGATIVE_FLAG);

    if (elOperando == 0)
        turnOnFlags(ZERO_FLAG);
    else
        turnOffFlags(ZERO_FLAG);

    return elOperando;
}

uint8_t opLsr(uint8_t elOperando) {

    if ((elOperando & 1) == 1)
        turnOnFlags(CARRY_FLAG);
    else
        turnOffFlags(CARRY_FLAG);

    elOperando >>= 1;

    if ((elOperando & 0x80) == 0x80)
        turnOnFlags(NEGATIVE_FLAG);
    else
        turnOffFlags(NEGATIVE_FLAG);

    if (elOperando == 0)
        turnOnFlags(ZERO_FLAG);
    else
        turnOffFlags(ZERO_FLAG);

    return elOperando;
}

void opBranch(bool condiccion) {

    if (condiccion) {
        uint16_t temp = cpu.PC;
        cpu.PC = (cpu.PC + 2 + (int8_t)getValImmed()) & 0xffff;

        if (((temp ^ cpu.PC) & 0x100) == 0x100)
            cpu.currentCycles += 2;
        else
            cpu.currentCycles++;

    }
    else
        cpu.PC += 2;

}

void opJmpInd() {

    if (getValImmed() == 0xff)
        cpu.PC = mapper.ioRead(getAddr_Abs()) + (mapper.ioRead(getAddr_Abs() - 0xff) << 8);
    else
        cpu.PC = mapper.ioRead(getAddr_Abs()) + (mapper.ioRead(getAddr_Abs() + 1) << 8);

}

void opJsr() {
    pushByte((cpu.PC + 2) >> 8);
    pushByte((cpu.PC + 2) & 0xFF);
    cpu.PC = getAddr_Abs();
}

void opRti() {
    cpu.flagsRegister = pullByte(0) & 0xef;
    cpu.PC = pullByte(0) | (pullByte(0) << 8);
}

void opRts() {
    cpu.PC = pullByte(0) | (pullByte(0) << 8);
    cpu.PC++;
}

void opInterrupt(uint16_t elPc, uint16_t addr, uint8_t flagsRegister) {
    cpu.updateInterruptCycles = true;
    pushByte((elPc) >> 8);
    pushByte((elPc) & 0xFF);
    pushByte(cpu.flagsRegister);
    turnOnFlags(INTERRUPT_FLAG);
    cpu.PC = addr;
}

void handleInterrupts() {

    if((ElMapper_GetInterruptSignal() & 8) == INTERRUPT_NMI) {	// NMI
        if (ElMapper_GetNmiDelayFlag()) {
            ElMapper_SetNmiDelayFlag(false);
            LaPpu_TurnOnFlags(NMI_FLAG);
        }
        else {
            ElMapper_ClearInterruptSignal(INTERRUPT_NMI);
            opInterrupt(cpu.PC, ElMapper_GetNmiAddr(), (cpu.flagsRegister & 0xef) | BIT_5);
        }
    }

    if(ElMapper_IrqIsRequested()) {	// IRQ

        if (ElMapper_GetIrqDelayFlag()) {
            ElMapper_SetIrqDelayFlag(false);
            return;
        }
        else if (!flagIsSet(INTERRUPT_FLAG)) {

            if((ElMapper_GetInterruptSignal() & 1) == INTERRUPT_FRAME_IRQ) { // TODO CAMBIAR POR SWITCH ??
                ElMapper_ClearInterruptSignal(INTERRUPT_FRAME_IRQ);
                opInterrupt(cpu.PC, ElMapper_GetIrqAddr(), (cpu.flagsRegister & 0xef) | BIT_5);
                return;
            }
            else if((ElMapper_GetInterruptSignal() & 2) == INTERRUPT_DMC_IRQ) {
                ElMapper_ClearInterruptSignal(INTERRUPT_DMC_IRQ);
                opInterrupt(cpu.PC, ElMapper_GetIrqAddr(), (cpu.flagsRegister & 0xef) | BIT_5);
                return;
            }
            else if((ElMapper_GetInterruptSignal() & 4) == INTERRUPT_MAPPER_IRQ) {
                ElMapper_ClearInterruptSignal(INTERRUPT_MAPPER_IRQ);
                opInterrupt(cpu.PC, ElMapper_GetIrqAddr(), (cpu.flagsRegister & 0xef) | BIT_5);
                return;
            }

        }
    }
}

uint16_t LaCpu_GetCurrentCycles() {
    return cpu.currentCycles;
}

uint16_t LaCpu_GetCurrentOpCycles() {
    return cpu.currentOpCycles;
}

void LaCpu_UpdateCurrentCycles(uint16_t cycles) {
    cpu.currentCycles += cycles;
}

uint16_t LaCpu_ExecuteInstruction() {

    if(ElMapper_GetInterruptSignal() > 0)
        handleInterrupts();

    cpu.currentOpcode = mapper.ioRead(cpu.PC);
    cpu.totalInstruc++;

    cpu.deltaCycles = cpu.currentCycles;
    cpu.currentOpCycles = cpu.opcodeCycles[cpu.currentOpcode];
    cpu.currentCycles += cpu.currentOpCycles;

    switch (cpu.currentOpcode) {
        case 0x00: { opInterrupt((cpu.PC + 2) & 0xFFFF, ElMapper_GetIrqAddr(), cpu.flagsRegister | BREAK_FLAG | BIT_5); break; }
        case 0x01: { opOr(mapper.ioRead(getAddr_Idx_Ind())); cpu.PC += 2; break; }
        case 0x04: { cpu.PC += 2; break; }

        case 0x05: { opOr(mapper.ioRead(getValImmed())); cpu.PC += 2; break; }
        case 0x06: { mapper.ioWrite(getValImmed(), opAsl(mapper.ioRead(getValImmed()))); cpu.PC += 2; break; }

        case 0x08: { pushByte(cpu.flagsRegister | BREAK_FLAG | 0x20); cpu.PC++; break; }
        case 0x09: { opOr(getValImmed()); cpu.PC += 2; break; }
        case 0x0A: { setRegA(opAsl(cpu.regA)); cpu.PC++; break; }

        case 0x0D: { opOr(mapper.ioRead(getAddr_Abs())); cpu.PC += 3; break; }
        case 0x0E: { mapper.ioWrite(getAddr_Abs(), opAsl(mapper.ioRead(getAddr_Abs()))); cpu.PC += 3; break; }
        case 0x10: { opBranch(!flagIsSet(NEGATIVE_FLAG)); break; }
        case 0x11: { checkBugIndIdx(); opOr(mapper.ioRead(getAddr_Ind_Idx())); cpu.PC += 2; break; }

        case 0x15: { opOr(mapper.ioRead((getValImmed() + cpu.regX) & 0xFF)); cpu.PC += 2; break; }
        case 0x16: { mapper.ioWrite((getValImmed() + cpu.regX) & 0xff, opAsl(mapper.ioRead((getValImmed() + cpu.regX) & 0xff))); cpu.PC += 2; break; }

        case 0x18: { turnOffFlags(CARRY_FLAG); cpu.PC++; break; }
        case 0x19: { checkBugAbsInd(cpu.regY); opOr(mapper.ioRead((getAddr_Abs() + cpu.regY) & 0xffff)); cpu.PC += 3; break; }
        case 0x1A: { cpu.PC++; break; }

        case 0x1D: { checkBugAbsInd(cpu.regX); opOr(mapper.ioRead((getAddr_Abs() + cpu.regX) & 0xffff)); cpu.PC += 3; break; }
        case 0x1E: { mapper.ioWrite((getAddr_Abs() + cpu.regX) & 0xffff, opAsl(mapper.ioRead((getAddr_Abs() + cpu.regX) & 0xffff))); cpu.PC += 3; break; }

        case 0x20: { opJsr(); break; }
        case 0x21: { opAnd(mapper.ioRead(getAddr_Idx_Ind())); cpu.PC += 2; break; }

        case 0x24: { opBit(mapper.ioRead(getValImmed())); cpu.PC += 2; break; }
        case 0x25: { opAnd(mapper.ioRead(getValImmed())); cpu.PC += 2; break; }
        case 0x26: { mapper.ioWrite(getValImmed(), opRol(mapper.ioRead(getValImmed()))); cpu.PC += 2;  break; }
        case 0x28: { cpu.flagsRegister = pullByte(1) & 0xef; ElMapper_SetIrqDelayFlag(true); break; }
        case 0x29: { opAnd(getValImmed()); cpu.PC += 2; break; }
        case 0x2A: { setRegA(opRol(cpu.regA)); cpu.PC++; break; }
        case 0x2C: { opBit(mapper.ioRead(getAddr_Abs())); cpu.PC += 3; break; }
        case 0x2D: { opAnd(mapper.ioRead(getAddr_Abs())); cpu.PC += 3; break; }
        case 0x2E: { mapper.ioWrite(getAddr_Abs(), opRol(mapper.ioRead(getAddr_Abs()))); cpu.PC += 3; break; }

        case 0x30: { opBranch(flagIsSet(NEGATIVE_FLAG)); break; }
        case 0x31: { checkBugIndIdx(); opAnd(mapper.ioRead(getAddr_Ind_Idx())); cpu.PC += 2; break; }
        case 0x35: { opAnd(mapper.ioRead((getValImmed() + cpu.regX) & 0xFF)); cpu.PC += 2; break; }
        case 0x36: { mapper.ioWrite((getValImmed() + cpu.regX) & 0xff, opRol(mapper.ioRead((getValImmed() + cpu.regX) & 0xff))); cpu.PC += 2; break; }
        case 0x38: { turnOnFlags(CARRY_FLAG); cpu.PC++; break; }
        case 0x39: { checkBugAbsInd(cpu.regY); opAnd(mapper.ioRead((getAddr_Abs() + cpu.regY) & 0xffff)); cpu.PC += 3; break; }
        case 0x3D: { checkBugAbsInd(cpu.regX); opAnd(mapper.ioRead((getAddr_Abs() + cpu.regX) & 0xffff)); cpu.PC += 3; break; }
        case 0x3E: { mapper.ioWrite((getAddr_Abs() + cpu.regX) & 0xffff, opRol(mapper.ioRead((getAddr_Abs() + cpu.regX) & 0xffff))); cpu.PC += 3; break; }
        case 0x40: { opRti(); break; }
        case 0x41: { opEor(mapper.ioRead(getAddr_Idx_Ind())); cpu.PC += 2; break; }
        case 0x45: { opEor(mapper.ioRead(getValImmed())); cpu.PC += 2; break; }
        case 0x46: { mapper.ioWrite(getValImmed(), opLsr(mapper.ioRead(getValImmed()))); cpu.PC += 2;  break; }
        case 0x48: { pushByte(cpu.regA); cpu.PC++; break; }
        case 0x49: { opEor(getValImmed()); cpu.PC += 2; break; }
        case 0x4A: { setRegA(opLsr(cpu.regA)); cpu.PC++; break; }
        case 0x4C: { cpu.PC = getAddr_Abs(); break; }
        case 0x4D: { opEor(mapper.ioRead(getAddr_Abs())); cpu.PC += 3; break; }
        case 0x4E: { mapper.ioWrite(getAddr_Abs(), opLsr(mapper.ioRead(getAddr_Abs()))); cpu.PC += 3; break; }
        case 0x50: { opBranch(!flagIsSet(OVERFLOW_FLAG)); break; }
        case 0x51: { checkBugIndIdx(); opEor(mapper.ioRead(getAddr_Ind_Idx())); cpu.PC += 2; break; }
        case 0x55: { opEor(mapper.ioRead((getValImmed() + cpu.regX) & 0xFF)); cpu.PC += 2; break; }
        case 0x56: { mapper.ioWrite((getValImmed() + cpu.regX) & 0xff, opLsr(mapper.ioRead((getValImmed() + cpu.regX) & 0xff))); cpu.PC += 2; break; }
        case 0x58: { turnOffFlags(INTERRUPT_FLAG); ElMapper_SetIrqDelayFlag(true); cpu.PC++; break; }
        case 0x59: { checkBugAbsInd(cpu.regY); opEor(mapper.ioRead((getAddr_Abs() + cpu.regY) & 0xffff)); cpu.PC += 3; break; }
        case 0x5A: { cpu.PC++; break; }

        case 0x5D: { checkBugAbsInd(cpu.regX); opEor(mapper.ioRead((getAddr_Abs() + cpu.regX) & 0xffff)); cpu.PC += 3; break; }
        case 0x5E: { mapper.ioWrite((getAddr_Abs() + cpu.regX) & 0xffff, opLsr(mapper.ioRead((getAddr_Abs() + cpu.regX) & 0xffff))); cpu.PC += 3; break; }
        case 0x60: { opRts(); break; }
        case 0x61: { opAdc(mapper.ioRead(getAddr_Idx_Ind())); cpu.PC += 2; break; }
        case 0x65: { opAdc(mapper.ioRead(getValImmed())); cpu.PC += 2; break; }
        case 0x66: { mapper.ioWrite(getValImmed(), opRor(mapper.ioRead(getValImmed()))); cpu.PC += 2;  break; }
        case 0x68: { setRegA(pullByte(1)); break; }
        case 0x69: { opAdc(getValImmed()); cpu.PC += 2; break; }
        case 0x6A: { setRegA(opRor(cpu.regA)); cpu.PC++; break; }
        case 0x6C: { opJmpInd(); break; }
        case 0x6D: { opAdc(mapper.ioRead(getAddr_Abs())); cpu.PC += 3; break; }
        case 0x6E: { mapper.ioWrite(getAddr_Abs(), opRor(mapper.ioRead(getAddr_Abs()))); cpu.PC += 3; break; }
        case 0x70: { opBranch(flagIsSet(OVERFLOW_FLAG)); break; }
        case 0x71: { checkBugIndIdx(); opAdc(mapper.ioRead(getAddr_Ind_Idx())); cpu.PC += 2; break; }
        case 0x75: { opAdc(mapper.ioRead((getValImmed() + cpu.regX) & 0xFF)); cpu.PC += 2; break; }
        case 0x76: { mapper.ioWrite((getValImmed() + cpu.regX) & 0xff, opRor(mapper.ioRead((getValImmed() + cpu.regX) & 0xff))); cpu.PC += 2; break; }
        case 0x78: { turnOnFlags(INTERRUPT_FLAG); ElMapper_SetIrqDelayFlag(true); cpu.PC++; break; }
        case 0x79: { checkBugAbsInd(cpu.regY); opAdc(mapper.ioRead((getAddr_Abs() + cpu.regY) & 0xffff)); cpu.PC += 3; break; }
        case 0x7A: { cpu.PC++; break;}

        case 0x7C: { cpu.PC += 3; break; }
        case 0x7D: { checkBugAbsInd(cpu.regX); opAdc(mapper.ioRead((getAddr_Abs() + cpu.regX) & 0xffff)); cpu.PC += 3; break; }
        case 0x7E: { mapper.ioWrite((getAddr_Abs() + cpu.regX) & 0xffff, opRor(mapper.ioRead((getAddr_Abs() + cpu.regX) & 0xffff))); cpu.PC += 3; break; }

        case 0x80: { cpu.PC+=2; break; }
        case 0x81: { mapper.ioWrite(getAddr_Idx_Ind(), cpu.regA); cpu.PC += 2; break; }
        case 0x84: { mapper.ioWrite(getValImmed(), cpu.regY); cpu.PC += 2; break; }
        case 0x85: { mapper.ioWrite(getValImmed(), cpu.regA); cpu.PC += 2; break; }
        case 0x86: { mapper.ioWrite(getValImmed(), cpu.regX); cpu.PC += 2; break; }
        case 0x88: { setRegY(opIncDec(cpu.regY, -1)); cpu.PC++; break; }
        case 0x8A: { setRegA(cpu.regX); cpu.PC++; break; }
        case 0x8C: { mapper.ioWrite(getAddr_Abs(), cpu.regY); cpu.PC += 3; break; }
        case 0x8D: { mapper.ioWrite(getAddr_Abs(), cpu.regA); cpu.PC += 3; break; }
        case 0x8E: { mapper.ioWrite(getAddr_Abs(), cpu.regX); cpu.PC += 3; break; }
        case 0x90: { opBranch(!flagIsSet(CARRY_FLAG)); break; }
        case 0x91: { mapper.ioWrite(getAddr_Ind_Idx(), cpu.regA); cpu.PC += 2; break; }
        case 0x94: { mapper.ioWrite((getValImmed() + cpu.regX) & 0xff, cpu.regY); cpu.PC += 2; break; }
        case 0x95: { mapper.ioWrite((getValImmed() + cpu.regX) & 0xff, cpu.regA); cpu.PC += 2; break; }
        case 0x96: { mapper.ioWrite((getValImmed() + cpu.regY) & 0xff, cpu.regX); cpu.PC += 2; break; }
        case 0x98: { setRegA(cpu.regY); cpu.PC++; break; }
        case 0x99: { mapper.ioWrite((getAddr_Abs() + cpu.regY) & 0xffff, cpu.regA); cpu.PC += 3; break; }
        case 0x9A: { cpu.regSP = cpu.regX; cpu.PC++; break; }
        case 0x9C: { opAndXY(cpu.regY, cpu.regX); cpu.PC += 3; break; }
        case 0x9E: { opAndXY(cpu.regX, cpu.regY); cpu.PC += 3; break; }
        case 0x9D: { mapper.ioWrite((getAddr_Abs() + cpu.regX) & 0xffff, cpu.regA); cpu.PC += 3; break; }
        case 0xA0: { setRegY(getValImmed()); cpu.PC += 2; break; }
        case 0xA1: { setRegA(mapper.ioRead(getAddr_Idx_Ind())); cpu.PC += 2; break; }
        case 0xA2: { setRegX(getValImmed()); cpu.PC += 2; break; }
        case 0xA4: { setRegY(mapper.ioRead(getValImmed())); cpu.PC += 2; break; }
        case 0xA5: { setRegA(mapper.ioRead(getValImmed())); cpu.PC += 2; break; }
        case 0xA6: { setRegX(mapper.ioRead(getValImmed())); cpu.PC += 2; break; }
        case 0xA8: { setRegY(cpu.regA); cpu.PC++; break; }
        case 0xA9: { setRegA(getValImmed()); cpu.PC += 2; break; }
        case 0xAA: { setRegX(cpu.regA); cpu.PC++; break; }
        case 0xAC: { setRegY(mapper.ioRead(getAddr_Abs())); cpu.PC += 3;; break; }
        case 0xAD: { setRegA(mapper.ioRead(getAddr_Abs())); cpu.PC += 3; break; }
        case 0xAE: { setRegX(mapper.ioRead(getAddr_Abs())); cpu.PC += 3; break; }
        case 0xB0: { opBranch(flagIsSet(CARRY_FLAG)); break; }
        case 0xB1: { checkBugIndIdx(); setRegA(mapper.ioRead(getAddr_Ind_Idx())); cpu.PC += 2; break; }
        case 0xB4: { setRegY(mapper.ioRead((getValImmed() + cpu.regX) & 0xFF)); cpu.PC += 2; break; }
        case 0xB5: { setRegA(mapper.ioRead((getValImmed() + cpu.regX) & 0xFF)); cpu.PC += 2; break; }
        case 0xB6: { setRegX(mapper.ioRead((getValImmed() + cpu.regY) & 0xFF)); cpu.PC += 2; break; }
        case 0xB8: { turnOffFlags(OVERFLOW_FLAG); cpu.PC++; break; }
        case 0xB9: { checkBugAbsInd(cpu.regY); setRegA(mapper.ioRead((getAddr_Abs() + cpu.regY) & 0xffff)); cpu.PC += 3; break; }
        case 0xBA: { setRegX(cpu.regSP); cpu.PC++; break; }
        case 0xBC: { checkBugAbsInd(cpu.regX); setRegY(mapper.ioRead((getAddr_Abs() + cpu.regX) & 0xffff)); cpu.PC += 3; break; }
        case 0xBD: { checkBugAbsInd(cpu.regX); setRegA(mapper.ioRead((getAddr_Abs() + cpu.regX) & 0xffff)); cpu.PC += 3; break; }
        case 0xBE: { checkBugAbsInd(cpu.regY); setRegX(mapper.ioRead((getAddr_Abs() + cpu.regY) & 0xffff)); cpu.PC += 3; break; }
        case 0xC0: { opCpy(getValImmed()); cpu.PC += 2; break; }
        case 0xC1: { opCmp(mapper.ioRead(getAddr_Idx_Ind())); cpu.PC += 2; break; }
        case 0xC4: { opCpy(mapper.ioRead(getValImmed())); cpu.PC += 2; break; }
        case 0xC5: { opCmp(mapper.ioRead(getValImmed())); cpu.PC += 2; break; }
        case 0xC6: { mapper.ioWrite(getValImmed(), opIncDec(mapper.ioRead(getValImmed()), -1)); cpu.PC += 2;  break; }
        case 0xC8: { setRegY(opIncDec(cpu.regY, 1)); cpu.PC++; break;  }
        case 0xC9: { opCmp(getValImmed()); cpu.PC += 2; break; }
        case 0xCA: { setRegX(opIncDec(cpu.regX, -1)); cpu.PC++; break; }
        case 0xCC: { opCpy(mapper.ioRead(getAddr_Abs())); cpu.PC += 3; break; }
        case 0xCD: { opCmp(mapper.ioRead(getAddr_Abs())); cpu.PC += 3; break;  }
        case 0xCE: { mapper.ioWrite(getAddr_Abs(), opIncDec(mapper.ioRead(getAddr_Abs()), -1)); cpu.PC += 3; break; }
        case 0xD0: { opBranch(!flagIsSet(ZERO_FLAG)); break; }
        case 0xD1: { checkBugIndIdx(); opCmp(mapper.ioRead(getAddr_Ind_Idx())); cpu.PC += 2;  break; }

        case 0xD4: { cpu.PC += 2; break; }
        case 0xD5: { opCmp(mapper.ioRead((getValImmed() + cpu.regX) & 0xFF)); cpu.PC += 2; break; }
        case 0xD6: { mapper.ioWrite((getValImmed() + cpu.regX) & 0xff, opIncDec(mapper.ioRead((getValImmed() + cpu.regX) & 0xff), -1)); cpu.PC += 2; break; }
        case 0xD8: { turnOffFlags(DECIMAL_FLAG); cpu.PC++; break; }
        case 0xD9: { checkBugAbsInd(cpu.regY); opCmp(mapper.ioRead((getAddr_Abs() + cpu.regY) & 0xffff)); cpu.PC += 3; break; }
        case 0xDA: { cpu.PC++; break; }
        case 0xDD: { checkBugAbsInd(cpu.regX); opCmp(mapper.ioRead((getAddr_Abs() + cpu.regX) & 0xffff)); cpu.PC += 3; break; }
        case 0xDE: { mapper.ioWrite((getAddr_Abs() + cpu.regX) & 0xffff, opIncDec(mapper.ioRead((getAddr_Abs() + cpu.regX) & 0xffff), -1)); cpu.PC += 3; break; }
        case 0xE0: { opCpx(getValImmed()); cpu.PC += 2; break; }
        case 0xE1: { opSbc(mapper.ioRead(getAddr_Idx_Ind())); cpu.PC += 2; break; }
        case 0xE2: { cpu.PC+=2; break; }

        case 0xE4: { opCpx(mapper.ioRead(getValImmed())); cpu.PC += 2; break; }
        case 0xE5: { opSbc(mapper.ioRead(getValImmed())); cpu.PC += 2; break; }
        case 0xE6: { mapper.ioWrite(getValImmed(), opIncDec(mapper.ioRead(getValImmed()), 1)); cpu.PC += 2;  break; }
        case 0xE8: { setRegX(opIncDec(cpu.regX, 1)); cpu.PC++; break; }
        case 0xE9: { opSbc(getValImmed()); cpu.PC += 2; break; }
        case 0xEA: { cpu.PC++; break; }
        case 0xEC: { opCpx(mapper.ioRead(getAddr_Abs())); cpu.PC += 3; break; }
        case 0xED: { opSbc(mapper.ioRead(getAddr_Abs())); cpu.PC += 3; break; }
        case 0xEE: { mapper.ioWrite(getAddr_Abs(), opIncDec(mapper.ioRead(getAddr_Abs()), 1)); cpu.PC += 3; break; }
        case 0xF0: { opBranch(flagIsSet(ZERO_FLAG)); break; }
        case 0xF1: { checkBugIndIdx(); opSbc(mapper.ioRead(getAddr_Ind_Idx())); cpu.PC += 2; break; }
        case 0xF5: { opSbc(mapper.ioRead((getValImmed() + cpu.regX) & 0xFF)); cpu.PC += 2; break; }
        case 0xF6: { mapper.ioWrite((getValImmed() + cpu.regX) & 0xff, opIncDec(mapper.ioRead((getValImmed() + cpu.regX) & 0xff), 1)); cpu.PC += 2; break; }
        case 0xF8: { turnOnFlags(DECIMAL_FLAG); cpu.PC++; break; }
        case 0xF9: { checkBugAbsInd(cpu.regY); opSbc(mapper.ioRead((getAddr_Abs() + cpu.regY) & 0xffff)); cpu.PC += 3; break; }
        case 0xFA: { cpu.PC++; break; }
        case 0xFD: { checkBugAbsInd(cpu.regX); opSbc(mapper.ioRead((getAddr_Abs() + cpu.regX) & 0xffff)); cpu.PC += 3; break; }
        case 0xFE: { mapper.ioWrite((getAddr_Abs() + cpu.regX) & 0xffff, opIncDec(mapper.ioRead((getAddr_Abs() + cpu.regX) & 0xffff), 1)); cpu.PC += 3; break; }
        case 0xFF: {
                     uint temp = opIncDec(mapper.ioRead((getAddr_Abs() + cpu.regX) & 0xffff), 1);
                     mapper.ioWrite((getAddr_Abs() + cpu.regX) & 0xffff, temp);
                     opSbc(temp);
                     cpu.PC += 3;
                     break;
        }
        default:
            printf("Invalid Opcode: %x \n", cpu.currentOpcode);
            break;
    }

    if(cpu.updateInterruptCycles) {
        cpu.currentCycles+=7;
        cpu.updateInterruptCycles = false;
    }

    if(mapper.updateDmcCycles) {
        cpu.currentCycles+=4;
        mapper.updateDmcCycles = false;
    }

    if(mapper.updateDmaCycles) {
        cpu.currentCycles+=513;
        mapper.updateDmaCycles = false;
    }

    return cpu.currentCycles - cpu.deltaCycles;

}

