#ifndef _SUPER_CHARGER
#define _SUPER_CHARGER

#include "LaConsola.h"
#include <SDL2/SDL.h>

#define FRAMEBUFFER_SIZE 786
#define CYCLES_PER_LINE 114
#define CYCLES_PER_SAMPLE 37

#define APU_FRAME_CYCLES 7467
#define SQUARE1 0
#define SQUARE2 1
#define TRIANGLE 2

#define NOISE 3

#define NOISE_REGISTER_0 0
#define NOISE_REGISTER_1 1
#define TRIANGLE_REGISTER_0 0
#define TRIANGLE_REGISTER_1 1
#define TRIANGLE_REGISTER_2 2
#define TRIANGLE 2
#define DMC 4
#define DMC_REGISTER_0 0
#define DMC_REGISTER_2 1
#define DMC_REGISTER_3 2

struct LaApu {
    int32_t* channelOutput;            // LA AMUESTRA FINAL PARA PlayR
    int32_t* channelSwitch;            // LIGA-DESLIGA CADA channel POR 2015 (1  = ACTIVADO)
    int32_t* channelLengthCounter;       // D7-3 del respectivo Register
    int32_t* channelTimer;               // BITS 2-0 DE $4003 (MSB) y BITS 8-0 DE $4002 (LSB) Counter DECREMENTAR
    int32_t* lengthCounterHaltFlag;    // D5 de 4000 (HALT COUNTER Y TAMBIEN LOOP ENVELOPE)
    int32_t* sqrLoopEnvelope;          // LOOP ENVELOPE FLAG DE SQUARE WAVES
    int32_t* sqrVolume;              // VOLUME DEL channel (BITS 3-0 DE $4000) - MULTIPLICAR POR 2048
    int32_t* sqrVolEnvelope;         // EL VOLUME ENVELOPADO
    int32_t* sqrEnvelopeCounter;      // EL COUNTER INTERNO DE SQUARE 1 ENVELOPE
    int32_t* sqrDutyActual;            // FORMATO DE LA Wave (BITS 7-6 DE $4000), MULTIPLICA POR 8 PARA LA POSICION EN ARRAY
    int32_t* sqrDutyOffset;            // LA POSICION ACTUAL DENTRO LOS 8 PASITOS DE LA Wave
    bool* constVolSqrFlag;          // D4 de 4000
    bool* RestartSqrEnv;         // ESCRIPTA EN 4003 HACE RESET DE ENVELOPE COUNTERS CUANDO NEL 1ER CLOCK DE FRAME COUNTER
    int32_t* sqrTimerReload;           // contiene el balor de registers de timer para recargar el Counter
    bool* sqrSweepEnableFlag;       // Bit 7 del Sweep register
    bool* sqrSweepReloadFlag;       // Setado por escripta a 4001
    bool* sqrSweepNegFlag;          // D3 de Sweep register
    int32_t* sqrSweepShift;            // El Shift (D2 - 0 de sweep register)
    int32_t* sqrSweepCounter;          // el Counter de sweep
    int32_t*  sqrSweepCounterReload;    // el Counter de sweep contenido en registro 4001
    int32_t* sqrRegister0;    			// el Counter de sweep contenido en registro 4001

    int32_t linearCounterHaltFlag;       // Halt del Linear Counter del Triangle
    int32_t linearCounterControlFlag;    // Control Flag (D7 de 4008)
    int32_t trgLinearCounter;            // EL Counter LINEAR DE TRIANGLE
    int32_t trgWaveOffset;             // LA POSICION EN EL SECUENCER, DE 0 AL 31
    int32_t* trgRegisters;    			// el Counter de sweep contenido en registro 4001

    int32_t noiseLengthCounterHalt;
    int32_t noiseConstVolumeFlag;
    int32_t noiseVolume;
    int32_t noiseVolumeEnvelope;
    int32_t noiseTimer;
    int32_t noiseLengthCounter;
    bool noiseRestartEnv;
    int32_t noiseEnvelopeCounter;
    int32_t noiseShiftRegister;
    int32_t noiseFeedbackBit;
    int32_t noiseModeRegister;          // si set vale 6, si reset vale 1
    int32_t noiseOutput;
    int32_t* noiseRegisters;    			// el Counter de sweep contenido en registro 4001

    int32_t dmcLoopFlag;                 // D6 d 4010
    int32_t dmcTimer;                    // valor de lookup definido por D0-3 de 4000 / 8
    int32_t dmcSampleBuffer;               // valor recibido de DMA y tambien shift register
    int32_t dmcDac = 0;                      // el counter de 7 bits y tambien audio output
    int32_t dmcShiftRemainBits;          // intern bit counter de shift register, cuando 7 empuja nuevo byte de memoria
    int32_t dmcRemainSamples;           // counter de bytes de la amuestra, setado por 4013
    int32_t dmcSampleAddr;         // setado por 4012
    int32_t* dmcRegisters;    			// el Counter de sweep contenido en registro 4001
    int32_t frameCounterPasitos;         // Numero de pasitos a dar D7 $2017 (0 = 4 pasitos, 1 = 5)
    int32_t frameCounterPasitosCounter; // el Counter interno del frame Counter
    int32_t frameCounter;               // incrementado por cpu cycles, cuando = 7457 clock el Counter y volta al 0
    int32_t frameCounterIrqInhFlag;      // D6 de 4017
    int32_t sampleCounter;             // Counter usado para seleccionar una cada 38 amuestras

	short* frameBuffer;
	int32_t frameBufferWriteCursor;
	int32_t frameBufferLecturaCursor;
	int32_t frameIrqFlag;
	int32_t dmcInterruptEnable;          // D7 de 4010
	int32_t dmcIrqFlag;
	bool dmcSilenceFlag;
	float* pulse_table;
	float* tnd_table;
    bool updateDmcCycles;

    SDL_AudioSpec audioSettings;

};

extern LaApu apu;

bool LaApu_Carga();
void LaApu_AudioUpdate();
void LaApu_audioTocaFrame();
void LaApu_ToggleChannel(int elBalor);
void LaApu_SetDmcIrqFlag(int value);
void LaApu_SqrRegister0Write(int canalNum, int value);
void LaApu_SqrRegister1Write(int canalNum, int value);
void LaApu_SqrRegister2Write(int canalNum, int value);
void LaApu_SqrRegister3Write(int canalNum, int value);
void LaApu_TrgRegister0Write(int value);
void LaApu_TrgRegister1Write(int value);
void LaApu_TrgRegister2Write(int value);
void LaApu_NoiseRegister0Write(int value);
void LaApu_NoiseRegister1Write(int value);
void LaApu_NoiseRegister2Write(int value);
void LaApu_DmcRegister0Write(int value);
void LaApu_DmcRegister1Write(int value);
void LaApu_DmcRegister2Write(int value);
void LaApu_DmcRegister3Write(int value);
void LaApu_Reg4017Write(int value);
int LaApu_Reg4015Read();
void LaApu_Descarga();

#endif
