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
#ifndef RTC_H
#define RTC_H	
    
#include "gps.h"

void rtcUpdate(gps_timestamp_t timestamp);
gps_timestamp_t rtcGetTimestamp();
    
#endif    
/* [] END OF FILE */
