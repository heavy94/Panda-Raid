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

#ifndef GPS_H
#define GPS_H
    
#include "project.h"

#define TRUE 1
#define FALSE 0

//NMEA sentence types
#define NMEA_GGA "GPGGA"

//Public functions
uint8 gps_receiveData(char c);
void gps_getTime(uint8 * hour, uint8 * min, uint8 * sec, uint8 * sec_d);

#endif
/* [] END OF FILE */
