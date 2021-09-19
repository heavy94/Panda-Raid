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
#ifndef BUTTONS_H
#define BUTTONS_H
    
#include "project.h"
    
void initButtons(uint16_t refresh_rate);
void updateButtons();

uint8_t getButtonShort_0();
uint8_t getButtonShort_1();

uint8_t getButtonLong_0();
uint8_t getButtonLong_1();
    
#endif
/* [] END OF FILE */
