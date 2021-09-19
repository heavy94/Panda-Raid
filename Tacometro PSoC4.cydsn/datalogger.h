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
#ifndef DATALOGGER_H
#define DATALOGGER_H
    
#include "project.h"
#include "rtc.h"
    
uint8 dataloggerStart();
void dataloggerStop();
void dataloggerSaveData(gps_data_t data);
uint8 dataloggerGetStatus();
    
#endif

/* [] END OF FILE */
