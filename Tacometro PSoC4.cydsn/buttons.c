/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "buttons.h"

#define BTN_NO_PRESS    0
#define BTN_SHORT_PRESS 1
#define BTN_LONG_PRESS  2
    
#define LONG_PRESS_TIME  3000 // ms
#define COUNTER_STOP    -1

uint8_t button_state_0;
uint8_t button_state_1;

uint8_t button_internal_0;
uint8_t button_internal_1;

int16_t counter_button_0;
int16_t counter_button_1;

uint16_t long_press_count;

// Función que inicializa todos los estados internos para la detección de las pulsaciones
// de los botones.
void initButtons(uint16_t refresh_rate)
{
    button_state_0 = BTN_NO_PRESS;
    button_state_1 = BTN_NO_PRESS;
    
    button_internal_0 = CyPins_ReadPin(BTN_0);
    button_internal_1 = CyPins_ReadPin(BTN_1);
    
    counter_button_0 = COUNTER_STOP;
    counter_button_1 = COUNTER_STOP;
    
    long_press_count = LONG_PRESS_TIME/refresh_rate;
}

// Función que se encarga de gestionar las pulsaciones de los botones, distinguiendo 
// entre pulsaciones largas y cortas. Los estados de las pulsaciones se limpian en 
// cada llamada de la función.
void updateButtons() // 'refresh_rate' must be less than 'LONG_PRESS_TIME'
{
    uint8_t new_button_state;
    
    // Clear all detected button pressings
    button_state_0 = BTN_NO_PRESS;
    button_state_1 = BTN_NO_PRESS;
    
    // Button 0
    if (counter_button_0 == 0) // Long press timing expired (Long press)
    {
        button_state_0 = BTN_LONG_PRESS;
        counter_button_0 = COUNTER_STOP;
    }
    else if (counter_button_0 > 0)
    {
        counter_button_0--;
    }
    
    new_button_state = CyPins_ReadPin(BTN_0);
    if (new_button_state != button_internal_0)
    {
        if (new_button_state == 0)  // Button pressed
        {
            counter_button_0 = long_press_count;
        }
        else if (counter_button_0 > 0) // Button released before long press time (Short press)
        {
            button_state_0 = BTN_SHORT_PRESS;
            counter_button_0 = COUNTER_STOP;
        }
    }
    button_internal_0 = new_button_state;
    
    // Button 1
    if (counter_button_1 == 0) // Long press timing expired (Long press)
    {
        button_state_1 = BTN_LONG_PRESS;
        counter_button_1 = COUNTER_STOP;
    }
    else if (counter_button_1 > 0)
    {
        counter_button_1--;
    }
    
    new_button_state = CyPins_ReadPin(BTN_1);
    if (new_button_state != button_internal_1)
    {
        if (new_button_state == 0)  // Button pressed
        {
            counter_button_1 = long_press_count;
        }
        else if (counter_button_1 > 0) // Button released before long press time (Short press)
        {
            button_state_1 = BTN_SHORT_PRESS;
            counter_button_1 = COUNTER_STOP;
        }
    }
    button_internal_1 = new_button_state;
}

uint8_t getButtonShort_0()
{
    return (button_state_0 == BTN_SHORT_PRESS)?1:0;
}

uint8_t getButtonShort_1()
{
    return (button_state_1 == BTN_SHORT_PRESS)?1:0;
}

uint8_t getButtonLong_0()
{
    return (button_state_0 == BTN_LONG_PRESS)?1:0;
}

uint8_t getButtonLong_1()
{
    return (button_state_1 == BTN_LONG_PRESS)?1:0;
}

/* [] END OF FILE */
