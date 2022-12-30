#include "LaConsola.h"

LosControls controls;

void turnOnButton(int32_t button) {
    controls.buttonFlags |= button;
}

void turnOffButton(int32_t button) {
    controls.buttonFlags = (controls.buttonFlags & (~(button))) & 0xff;
}

void LosControls_PresionaBoton(SDL_Event* elEventro) {
    switch(elEventro->key.keysym.sym) {
        case SDLK_z:    // BOTON B
            turnOnButton(BUTTON_B);
            break;
        case SDLK_x:    // BOTON A
            turnOnButton(BUTTON_A);
            break;
        case SDLK_a:    // BOTON SELECT
            turnOnButton(BUTTON_SELECT);
            break;
        case SDLK_s:    // BOTON START
            turnOnButton(BUTTON_START);
            break;
        case SDLK_UP:    // SETA ARRIBA
            turnOnButton(BUTTON_UP);
            break;
        case SDLK_DOWN:    // SETA ABAJO
            turnOnButton(BUTTON_DOWN);
            break;
        case SDLK_LEFT:    // SETA IZQUIERDA
            turnOnButton(BUTTON_LEFT);
            break;
        case SDLK_RIGHT:    // SETA DERECHA
            turnOnButton(BUTTON_RIGHT);
            break;
    }
}

void LosControls_SoltaBoton(SDL_Event* elEventro) {
    switch(elEventro->key.keysym.sym) {
        case SDLK_z:    // BOTON B
            turnOffButton(BUTTON_B);
            break;
        case SDLK_x:    // BOTON A
            turnOffButton(BUTTON_A);
            break;
        case SDLK_a:    // BOTON SELECT
            turnOffButton(BUTTON_SELECT);
            break;
        case SDLK_s:    // BOTON START
            turnOffButton(BUTTON_START);
            break;
        case SDLK_UP:    // SETA ARRIBA
            turnOffButton(BUTTON_UP);
            break;
        case SDLK_DOWN:    // SETA ABAJO
            turnOffButton(BUTTON_DOWN);
            break;
        case SDLK_LEFT:    // SETA IZQUIERDA
            turnOffButton(BUTTON_LEFT);
            break;
        case SDLK_RIGHT:    // SETA DERECHA
            turnOffButton(BUTTON_RIGHT);
            break;
    }
}

int32_t LosControls_Reg4016Read() {
    int32_t temp =  ((controls.buttonFlags >> controls.readCounter) & 1) | 0x40;
    controls.readCounter++;
    if (controls.readCounter == 24)
        controls.readCounter = 0;
    return temp;
}

void LosControls_Reg4016Write(int32_t value) {
    if (((value & 1) == 0) && ( controls.p1Strobe == 1))    // ESCRIBIR 1 y despues 0 reset y activa lectura
        controls.readCounter = 0;
    controls.p1Strobe = value & 1;
}

void LosControls_Reg4017Write(int value) {
    if (((value & 1) == 0) && ( controls.p2Strobe == 1))    // ESCRIBIR 1 y despues 0 reset y activa lectura
        controls.readCounter = 0;
    controls.p2Strobe = value & 1;
}
