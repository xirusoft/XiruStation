#include "LaConsola.h"

LaApu apu;

static const uint8_t dutyWaves[] = {
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1
};

static const uint8_t trgWaves[] = {
    0xF, 0xE, 0xD, 0xC, 0xB, 0xA, 9, 8, 7, 6,
    5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7,
    8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF
};

static const uint16_t lengthCounterValues[] = {
    0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
    0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
    0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
    0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E
};

static const uint16_t noiseFreq[] = {
    4, 8, 0x10, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xCA,
    0xFE, 0x17C, 0x1FC, 0x2FA, 0x3F8, 0x7F2, 0xFE4
};

static const uint16_t dmcTimerValues[] = {
    428, 380, 340, 320, 286, 254, 226, 214, 190,
    160, 142, 128, 106,  84,  72,  54

};

bool LaApu_Carga() {
    bool elSuceso = true;

    if(
        LaConsola_CargaMemoriaS32(&apu.channelOutput, 3) &
        LaConsola_CargaMemoriaS32(&apu.channelSwitch, 3) &
        LaConsola_CargaMemoriaS32(&apu.channelLengthCounter, 3) &
        LaConsola_CargaMemoriaS32(&apu.channelTimer, 3) &
        LaConsola_CargaMemoriaS32(&apu.lengthCounterHaltFlag, 3) &
        LaConsola_CargaMemoriaS32(&apu.sqrLoopEnvelope, 2) &
        LaConsola_CargaMemoriaS32(&apu.sqrVolume, 2) &
        LaConsola_CargaMemoriaS32(&apu.sqrVolEnvelope, 3) &
        LaConsola_CargaMemoriaS32(&apu.sqrEnvelopeCounter, 2) &
        LaConsola_CargaMemoriaS32(&apu.sqrDutyActual, 2) &
        LaConsola_CargaMemoriaS32(&apu.sqrDutyOffset, 2) &
        LaConsola_CargaMemoriaB(&apu.constVolSqrFlag, 2) &
        LaConsola_CargaMemoriaB(&apu.RestartSqrEnv, 2) &
        LaConsola_CargaMemoriaS32(&apu.sqrTimerReload, 2) &
        LaConsola_CargaMemoriaB(&apu.sqrSweepEnableFlag, 2) &
        LaConsola_CargaMemoriaB(&apu.sqrSweepReloadFlag, 2) &
        LaConsola_CargaMemoriaB(&apu.sqrSweepNegFlag, 2) &
        LaConsola_CargaMemoriaS32(&apu.sqrSweepShift, 2) &
        LaConsola_CargaMemoriaS32(&apu.sqrSweepCounter, 2) &
        LaConsola_CargaMemoriaS32(&apu.sqrSweepCounterReload, 2) &
        LaConsola_CargaMemoriaS32(&apu.sqrRegister0, 2) &
        LaConsola_CargaMemoriaS32(&apu.trgRegisters, 3) &
        LaConsola_CargaMemoriaS32(&apu.noiseRegisters, 2) &
        LaConsola_CargaMemoriaS32(&apu.dmcRegisters, 2) &
        LaConsola_CargaMemoriaS16(&apu.frameBuffer, FRAMEBUFFER_SIZE) &
        LaConsola_CargaMemoriaF(&apu.pulse_table, 31) &
        LaConsola_CargaMemoriaF(&apu.tnd_table, 203)
      ) {
            for(int n = 0; n < 31; n++)
                apu.pulse_table [n] = (float) (95.52 / (8128.0 / n + 100) * 0xffff);
            for(int n =0; n < 203; n++)
                apu.tnd_table [n] = (float) (163.67 / (24329.0 / n + 100) * 0xffff);

        }
    else {
        printf("error cargando CPU, arrola tu computadora de la puente de amistad !!!\n");
        elSuceso = false;
    }

    apu.frameCounterPasitos = 4;
    apu.frameCounterIrqInhFlag = 4;
    apu.noiseShiftRegister = 1;
    apu.noiseModeRegister = 1;
    apu.dmcShiftRemainBits = 7;
    apu.audioSettings.freq = 48000;
    apu.audioSettings.format = AUDIO_S16;
    apu.audioSettings.samples = 786;
    apu.audioSettings.channels = 1;
    SDL_OpenAudio(&apu.audioSettings, 0);

    return elSuceso;
}

void UpdataSqrEnvelope(int channelNum) {

    if (apu.RestartSqrEnv[channelNum]) {
        apu.sqrEnvelopeCounter[channelNum] = (apu.sqrRegister0[channelNum] & 0xF) + 1;
        apu.sqrVolEnvelope[channelNum] = 0xF;
        apu.RestartSqrEnv[channelNum] = false;
    }
    else {
        apu.sqrEnvelopeCounter[channelNum]--;

        if (apu.sqrEnvelopeCounter[channelNum] <= 0) {
            apu.sqrEnvelopeCounter[channelNum] = (apu.sqrRegister0[channelNum] & 0xF) + 1;

            if (apu.sqrVolEnvelope[channelNum] > 0)
                apu.sqrVolEnvelope[channelNum] -= 1;
            else
                apu.sqrVolEnvelope[channelNum] = (short) (apu.sqrLoopEnvelope[channelNum] == 0xFF ? 0xF : 0);
        }

    }

    if(apu.constVolSqrFlag[channelNum])
        apu.sqrVolume[channelNum] = (short) (apu.sqrRegister0[channelNum] & 0xF);
    else
        apu.sqrVolume[channelNum] = apu.sqrVolEnvelope[channelNum];

}

void updateNoiseEnvelope() {

    if (apu.noiseRestartEnv) {
        apu.noiseEnvelopeCounter = (apu.noiseRegisters[NOISE_REGISTER_0] & 0xF) + 1;
        apu.noiseVolumeEnvelope = 0xF;
        apu.noiseRestartEnv = false;
    }
    else {
        apu.noiseEnvelopeCounter--;

        if (apu.noiseEnvelopeCounter <= 0) {
            apu.noiseEnvelopeCounter = (apu.noiseRegisters[NOISE_REGISTER_0] & 0xF) + 1;

            if (apu.noiseVolumeEnvelope > 0)
                apu.noiseVolumeEnvelope -= 1;
            else
                apu.noiseVolumeEnvelope = 0;

        }

    }

    if(apu.noiseConstVolumeFlag == 1)
        apu.noiseVolume =  apu.noiseRegisters[NOISE_REGISTER_0] & 0xF;
    else
        apu.noiseVolume = apu.noiseVolumeEnvelope;
}

void updateLinearCounter() {

    if (apu.linearCounterHaltFlag == 1)
        apu.trgLinearCounter = apu.trgRegisters[TRIANGLE_REGISTER_0] & 0x7F;

    if ((apu.trgLinearCounter > 0) && (apu.linearCounterHaltFlag == 0))
        apu.trgLinearCounter--;

    if (apu.linearCounterControlFlag == 0)
        apu.linearCounterHaltFlag = 0;

}

void updateLengthCounters() {

    for (int n = 0; n < 3; n++) {

        if ((apu.lengthCounterHaltFlag[n] & apu.channelLengthCounter[n]) != 0)
            apu.channelLengthCounter[n]--;

    }

}

void updateNoiseLengthCounter()  {

    if ((apu.noiseLengthCounterHalt & apu.noiseLengthCounter) != 0)
        apu.noiseLengthCounter--;

}

void updateSquareSweep(int channelNum) {

    if (--apu.sqrSweepCounter[channelNum] < 0) {
        apu.sqrSweepCounter[channelNum] = apu.sqrSweepCounterReload[channelNum] + 1;

        if (apu.sqrSweepEnableFlag[channelNum] && apu.sqrTimerReload[channelNum] > 7 && apu.sqrSweepShift[channelNum] > 0) {

            int elOperando = apu.sqrTimerReload[channelNum] >> apu.sqrSweepShift[channelNum];

            if (apu.sqrSweepNegFlag[channelNum])
                elOperando = (~elOperando) & 0xFFF;

            if (channelNum == 1)
                elOperando = (elOperando + 1) & 0xFFF;

            apu.sqrTimerReload[channelNum] = (apu.sqrTimerReload[channelNum] + elOperando) & 0xFFF;

        }

    }

    if (apu.sqrSweepReloadFlag[channelNum]) {

        apu.sqrSweepCounter[channelNum] = apu.sqrSweepCounterReload[channelNum] + 1;
        apu.sqrSweepReloadFlag[channelNum] = false;

    }

}


void executeFrameCounter() {
    switch (apu.frameCounterPasitosCounter) {

        case 0: // PASITO 1
            UpdataSqrEnvelope(SQUARE1);
            UpdataSqrEnvelope(SQUARE2);
            updateNoiseEnvelope();
            updateLinearCounter();
            apu.frameCounterPasitosCounter++;
            break;

        case 1: // PASITO 2
            updateLengthCounters();
            updateNoiseLengthCounter();
            updateSquareSweep(SQUARE1);
            updateSquareSweep(SQUARE2);
            UpdataSqrEnvelope(SQUARE1);
            UpdataSqrEnvelope(SQUARE2);
            updateNoiseEnvelope();
            updateLinearCounter();
            apu.frameCounterPasitosCounter++;
            break;

        case 2: // PASITO 3
            UpdataSqrEnvelope(SQUARE1);
            UpdataSqrEnvelope(SQUARE2);
            updateNoiseEnvelope();
            updateLinearCounter();
            apu.frameCounterPasitosCounter++;
            break;

        case 3: // PASITO 4 - SE 5 PASITOS, PULA PARA EL PROXIMO

            if (apu.frameCounterPasitos == 4) {
                updateLengthCounters();
                updateNoiseLengthCounter();
                updateSquareSweep(SQUARE1);
                updateSquareSweep(SQUARE2);
                UpdataSqrEnvelope(SQUARE1);
                UpdataSqrEnvelope(SQUARE2);
                updateNoiseEnvelope();
                updateLinearCounter();

                if (apu.frameCounterIrqInhFlag == 0) {
                    apu.frameIrqFlag = 0x40;
                    ElMapper_SetInterruptSignal(INTERRUPT_FRAME_IRQ);
                }

                apu.frameCounterPasitosCounter = 0;
            }

            else
                apu.frameCounterPasitosCounter++;

            break;

        case 4:
            updateLengthCounters();
            updateNoiseLengthCounter();
            updateSquareSweep(SQUARE1);
            updateSquareSweep(SQUARE2);
            UpdataSqrEnvelope(SQUARE1);
            UpdataSqrEnvelope(SQUARE2);
            updateNoiseEnvelope();
            updateLinearCounter();
            apu.frameCounterPasitosCounter = 0;
            break;
    }
}

void updateTriangleWave() {

    apu.channelTimer[TRIANGLE] = (apu.trgRegisters[TRIANGLE_REGISTER_1] |
            (apu.trgRegisters[TRIANGLE_REGISTER_2] & 7) << 8) + 1;

    if ((apu.trgLinearCounter > 0) && (apu.channelLengthCounter[TRIANGLE] > 0)) {
        apu.trgWaveOffset = (apu.trgWaveOffset + 1) & 0x1F;

        // PERIODOS 0 O 1 CAUSAN HIGH PITCH SONIDOS - PEQUENAS FLATULENCIAS EN TELA TITULO DE F1 RACE, HAY QUE SILENCIAR EL channel
        if(apu.channelTimer[TRIANGLE] < 2)
                apu.channelOutput[TRIANGLE] =  0;
        else
            apu.channelOutput[TRIANGLE] = trgWaves[apu.trgWaveOffset] & apu.channelSwitch[TRIANGLE];
    }

}

void checkDmcAddrOverflow() {

    if (apu.dmcSampleAddr > 0xFFFF)
        apu.dmcSampleAddr = 0x8000;

}

void updateSampleBuffer() {

    if (apu.dmcRemainSamples > 0) {
        apu.dmcSampleBuffer = mapper.ioRead(apu.dmcSampleAddr++);
        apu.updateDmcCycles = true;
        apu.dmcSilenceFlag = false;
        apu.dmcRemainSamples--;
    }

    checkDmcAddrOverflow();

    if (apu.dmcRemainSamples == 0) {
        apu.dmcSilenceFlag = true;

        if (apu.dmcLoopFlag == 0x40) {
            apu.dmcSampleAddr = 0xC000 + (apu.dmcRegisters[DMC_REGISTER_2] * 64);
            apu.dmcRemainSamples = (apu.dmcRegisters[DMC_REGISTER_3] * 16) + 1;
        }
        else{

            if (apu.dmcInterruptEnable == 0x80)
                apu.dmcIrqFlag = 1;

        }

    }

}

void updateDmcWave() {

    if(apu.channelSwitch[DMC] == 0)
        return;

    apu.dmcTimer = dmcTimerValues[apu.dmcRegisters[DMC_REGISTER_0] & 0xF];

    if(apu.dmcSilenceFlag == false) {

        if ((apu.dmcSampleBuffer & 1) == 1)
            apu.dmcDac = (int8_t) ((apu.dmcDac + 1) < 0x7e ? apu.dmcDac + 2 : apu.dmcDac);
        else
            apu.dmcDac = (int8_t) ((apu.dmcDac - 1) > 1 ? apu.dmcDac - 2 : apu.dmcDac);

    }

    apu.dmcSampleBuffer >>= 1;

    if (--apu.dmcShiftRemainBits <=0) {

        apu.dmcShiftRemainBits = 8;
        updateSampleBuffer();

    }
    if(apu.dmcIrqFlag !=0) {
        ElMapper_SetInterruptSignal(INTERRUPT_DMC_IRQ);
        ElMapper_SetIrqDelayFlag(false);

    }
}

void updateSquareWaves(int channelNum) {

    if (apu.channelTimer[channelNum] < 0) {// cuando el Timer pasa de 0 a t, ** PLUS ONE *** cicla la Wave y restaura balor de timer
        apu.sqrDutyOffset[channelNum] = (apu.sqrDutyOffset[channelNum] + 1) & 0x7;
        apu.channelTimer[channelNum] = apu.sqrTimerReload[channelNum];// + 1;
    }

}
int elCounter = 0;
void updateSquareOutputs(int channelNum) {

    if ((apu.channelLengthCounter[channelNum] != 0) && (apu.channelTimer[channelNum] > 7) )
        apu.channelOutput[channelNum] =
        (dutyWaves[apu.sqrDutyActual[channelNum] + apu.sqrDutyOffset[channelNum]] * apu.sqrVolume[channelNum] & apu.channelSwitch[channelNum]);

    else {
        apu.channelOutput[channelNum] = 0;

    }
}

void updateNoiseWave() {

    if (apu.noiseTimer < 0) {// cuando el Timer pasa de 0 a t, ** PLUS ONE *** cicla la Wave y restaura balor de timer
        apu.noiseFeedbackBit = (apu.noiseShiftRegister & 1) ^ ((apu.noiseShiftRegister >> apu.noiseModeRegister) & 1);

        if(apu.noiseFeedbackBit == 1)
            apu.noiseShiftRegister = ((apu.noiseShiftRegister >> 1) | 0x4000);
        else
            apu.noiseShiftRegister = ((apu.noiseShiftRegister >> 1) & 0x3fff);

        apu.noiseTimer = noiseFreq[apu.noiseRegisters[NOISE_REGISTER_1] & 0xF];

    }

    if ((apu.noiseLengthCounter != 0) && ((apu.noiseShiftRegister & 1) != 0))
        apu.noiseOutput = apu.noiseVolume & apu.channelSwitch[NOISE];
    else
        apu.noiseOutput = 0;

}

void LaApu_AudioUpdate() {

    for (int n = 0; n < CYCLES_PER_LINE; n++) {  // loop 114 veces por scanline

        if (apu.frameCounter++ > APU_FRAME_CYCLES) {
            apu.frameCounter = 0;
            executeFrameCounter();
        }

        if (--apu.channelTimer[TRIANGLE] <= 0)
            updateTriangleWave();      // EL TIMER DE TRIANGLE DECREMENTA CADA CPU CYCLE

        if (--apu.dmcTimer < 0)
            updateDmcWave();

        if (n % 2 == 1) {         // LOS TIMES DE PULSE CADA 2ER CPU CYCLE
            apu.channelTimer[SQUARE1]--;
            apu.channelTimer[SQUARE2]--;
            apu.noiseTimer--;
        }
        updateSquareWaves(SQUARE1);
        updateSquareWaves(SQUARE2);
        updateNoiseWave();
        updateSquareOutputs(SQUARE1);
        updateSquareOutputs(SQUARE2);

        apu.sampleCounter++;

        if (apu.sampleCounter > CYCLES_PER_SAMPLE) { // OUTPUT UNA CADA 38 AMUESTRAS
            apu.sampleCounter = 0;
           int32_t pulse_out = apu.pulse_table[apu.channelOutput[SQUARE1] + apu.channelOutput[SQUARE2] ];
           int32_t tnd_out = apu.tnd_table [3 * apu.channelOutput[TRIANGLE] + 2 * apu.noiseOutput + apu.dmcDac];
           apu.frameBuffer[apu.frameBufferWriteCursor++] = 	(pulse_out + tnd_out) ^ 0x8000;// HAY QUE INVERTER EL SINAL DEL MSB PUES PODERA HABER OVERFLOR
            if (apu.frameBufferWriteCursor == FRAMEBUFFER_SIZE)
                apu.frameBufferWriteCursor = 0;
        }
    }
}

void LaApu_audioTocaFrame() {
    while(SDL_GetQueuedAudioSize(1) > 0) { }  // SINCRONIZACION DEL EMU POR APU

    void *bufa = malloc(FRAMEBUFFER_SIZE * sizeof(short));    // funcion malloc piede el talle en bytes - 2 bytes por amuestra
    short *bufaPtr = (short*)bufa;
    for(int n=0; n< FRAMEBUFFER_SIZE; n++)
        *bufaPtr++ = apu.frameBuffer[n];

    SDL_QueueAudio(1, bufa, FRAMEBUFFER_SIZE * 2); // Talle de bufa para queue en BYTES
    free(bufa);
    SDL_PauseAudio(0);
}

void LaApu_ToggleChannel(int elBalor) {
    for (int n = 0; n < 5; n++) {
        if (((elBalor >> n) & 1) == 1) // Bit set, activa el channel
            apu.channelSwitch[n] = 0x7FFFFFF;
        else {                           // Bit Reset, desactiva el channel; la desactivacion reseta el lengthCounter
            apu.channelSwitch[n] = 0;

            if(n < 3)
                apu.channelLengthCounter[n] = 0;
            else if(n == 3)
                apu.noiseLengthCounter = 0;
        }
    }

    if((elBalor & 0x10) == 0x10) {	// liga DMC
        apu.dmcSampleAddr = 0xC000 + (apu.dmcRegisters[DMC_REGISTER_2] * 64);
            apu.dmcRemainSamples = (apu.dmcRegisters[DMC_REGISTER_3] * 16) + 1;
    }
    else
        apu.dmcRemainSamples = 0;

}

void LaApu_SetDmcIrqFlag(int value) {
	  apu.dmcIrqFlag = value;
}

void LaApu_SqrRegister0Write(int canalNum, int value) {
    apu.sqrRegister0[canalNum] = value;

    if((value & 0x20) == 0x20) // en APU register bit 0 = FLAG LIGADO, 1 = DESLIGADO
         apu.lengthCounterHaltFlag[canalNum] = 0;
    else
         apu.lengthCounterHaltFlag[canalNum] = 0xff;

    apu.sqrLoopEnvelope[canalNum] = (~apu.lengthCounterHaltFlag[canalNum]) & 0xFF;
    apu.constVolSqrFlag[canalNum] = (value & 0x10) == 0x10;
    apu.sqrDutyActual[canalNum] = ((value & 0xC0) >> 6) * 8;

    if(apu.constVolSqrFlag[canalNum])
        apu.sqrVolume[canalNum] = (int16_t)(apu.sqrRegister0[canalNum] & 0xF);
    else
      apu.sqrVolume[canalNum] = (int16_t)(apu.sqrVolEnvelope[canalNum]);

}

void LaApu_SqrRegister1Write(int canalNum, int value) {
    apu.sqrSweepReloadFlag[canalNum] = true;
    apu.sqrSweepEnableFlag[canalNum] = (value & 0x80) >> 7 == 1;
    apu.sqrSweepNegFlag[canalNum] = (value & 0x8) >> 3 == 1;
    apu.sqrSweepShift[canalNum] = value & 7;
    apu.sqrSweepCounterReload[canalNum] = (value & 0x70) >> 4;
    updateSquareSweep(canalNum);

}

void LaApu_SqrRegister2Write(int canalNum, int value) {
    apu.sqrTimerReload[canalNum] = (apu.sqrTimerReload[canalNum] & 0x700) | value;
}

void LaApu_SqrRegister3Write(int canalNum, int value) {
    apu.sqrTimerReload[canalNum] = (apu.sqrTimerReload[canalNum] & 0xff) | ((value & 7) << 8);
    apu.channelLengthCounter[canalNum] = lengthCounterValues[value >> 3];
    apu.sqrDutyOffset[canalNum] = 0;
    apu.RestartSqrEnv[canalNum] = true;
}

void LaApu_TrgRegister0Write(int value) {
    apu.trgRegisters[TRIANGLE_REGISTER_0] = value;
    if((value & 0x80) == 0x80) {// en APU register bit 0 = FLAG LIGADO, 1 = DESLIGADO
        apu.lengthCounterHaltFlag[TRIANGLE] = 0;
        apu.linearCounterControlFlag = 1;
    }
    else {
        apu.lengthCounterHaltFlag[TRIANGLE] = 0xff;
        apu.linearCounterControlFlag = 0;
    }
}

void LaApu_TrgRegister1Write(int value) {
    apu.trgRegisters[TRIANGLE_REGISTER_1] = value;
    apu.channelTimer[TRIANGLE] = (apu.channelTimer[TRIANGLE] & 0x700) | value;
}

void LaApu_TrgRegister2Write(int value) {
    apu.trgRegisters[TRIANGLE_REGISTER_2] = value;
    apu.channelTimer[TRIANGLE] = ((apu.channelTimer[TRIANGLE] & 0xff) | ((value & 7) << 8)) + 1;
    apu.channelLengthCounter[TRIANGLE] = lengthCounterValues[value >> 3];
    apu.linearCounterHaltFlag = 1;
}

void LaApu_NoiseRegister0Write(int value) {
    apu.noiseRegisters[NOISE_REGISTER_0] = value;
    if((value & 0x20) == 0x20)
      apu.noiseLengthCounterHalt = 0;
    else
      apu.noiseLengthCounterHalt = 0xff;

    if((value & 0x10) == 0x10) {
      apu.noiseConstVolumeFlag = 1;
      apu.noiseVolume = value & 0xF;
    }
    else {
      apu.noiseConstVolumeFlag = 0;
      apu.noiseVolume = apu.noiseVolumeEnvelope;
    }
}

void LaApu_NoiseRegister1Write(int value) {
    apu.noiseRegisters[NOISE_REGISTER_1] = value;
    if((value & 0x80) == 0x80)
        apu.noiseModeRegister = 6;
    else
        apu.noiseModeRegister = 1;
    apu.noiseTimer = noiseFreq[value & 0xF];
}

void LaApu_NoiseRegister2Write(int value) {
    apu.noiseLengthCounter = lengthCounterValues[value >> 3];
    apu.noiseRestartEnv = true;
}

void LaApu_DmcRegister0Write(int value) {
    apu.dmcRegisters[DMC_REGISTER_0] = value;
    apu.dmcTimer = dmcTimerValues[value & 0xF];
    if((value & 0x80) == 0x80)
        apu.dmcInterruptEnable = 0x80;
    else {
        apu.dmcInterruptEnable = 0;
        apu.dmcIrqFlag = 0;
    }
    if((value & 0x40) == 0x40)
        apu.dmcLoopFlag = 0x40;
    else
        apu.dmcLoopFlag = 0;
}

void LaApu_DmcRegister1Write(int value) {
    apu.dmcDac = (int8_t) (value & 0x7f);
}

void LaApu_DmcRegister2Write(int value) {
    apu.dmcRegisters[DMC_REGISTER_2] = value;
    apu.dmcSampleAddr = 0xC000 + (value * 64);
}

void LaApu_DmcRegister3Write(int value) {
    apu.dmcRegisters[DMC_REGISTER_3] = value;
    apu.dmcRemainSamples = (value * 16) + 1;
}

void LaApu_Reg4017Write(int value) {
    if((value & 0x80) == 0x80)
        apu.frameCounterPasitos = 5;
    else
        apu.frameCounterPasitos = 4;
    if (apu.frameCounterPasitos == 4) {
        apu.frameCounterPasitosCounter = 0;
        apu.frameCounter = 0;
    }
    if((value & 0x40) == 0x40) {
        apu.frameCounterIrqInhFlag = 4;
        apu.frameIrqFlag = 0;
        ElMapper_ClearInterruptSignal(INTERRUPT_FRAME_IRQ);
    }
    else
        apu.frameCounterIrqInhFlag = 0;
}

int LaApu_Reg4015Read() {
    int temp = 0;
    for (int n = 0; n < 3; n++) {
        temp |= apu.channelLengthCounter[n] > 0 ? 1 << n : 0;
    }
    temp |= apu.noiseLengthCounter > 0 ? 8 : 0;
    temp |= apu.frameIrqFlag;
    apu.frameIrqFlag = 0;
    ElMapper_ClearInterruptSignal(INTERRUPT_FRAME_IRQ);
    if(apu.dmcRemainSamples > 0)
        temp |= 0x10;
    if(apu.dmcIrqFlag !=0)
        temp |= 0x80;
    return temp;
}

void LaApu_Descarga() {
    free(apu.channelOutput);
    free(apu.channelSwitch);
    free(apu.channelLengthCounter);
    free(apu.channelTimer);
    free(apu.lengthCounterHaltFlag);
    free(apu.sqrLoopEnvelope);
    free(apu.sqrVolume);
    free(apu.sqrVolEnvelope);
    free(apu.sqrEnvelopeCounter);
    free(apu.sqrDutyActual);
    free(apu.sqrDutyOffset);
    free(apu.constVolSqrFlag);
    free(apu.RestartSqrEnv);
    free(apu.sqrTimerReload);
    free(apu.sqrSweepEnableFlag);
    free(apu.sqrSweepReloadFlag);
    free(apu.sqrSweepNegFlag);
    free(apu.sqrSweepShift);
    free(apu.sqrSweepCounter);
    free(apu.sqrSweepCounterReload);
    free(apu.sqrRegister0);
    free(apu.trgRegisters);
    free(apu.noiseRegisters);
    free(apu.dmcRegisters);
    free(apu.frameBuffer);
    free(apu.pulse_table);
    free(apu.tnd_table);
}

