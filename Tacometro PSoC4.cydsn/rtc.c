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
#include "project.h"
#include "rtc.h"
#include "FatFs\diskio.h"
#include "FatFs\ff.h"

gps_timestamp_t rtc;

void rtcUpdate(gps_timestamp_t timestamp)
{
    rtc = timestamp;
}

gps_timestamp_t rtcGetTimestamp()
{
    return rtc;
}

// Function needed by FatFs library
DWORD get_fattime()
{
	/* Pack date and time into a DWORD variable */
	return (((DWORD)rtc.year + 20) << 25) | ((DWORD)rtc.month << 21) | ((DWORD)rtc.day << 16) | 
             (WORD)(rtc.hour << 11) | (WORD)(rtc.min << 5) | (WORD)(rtc.sec >> 1);
}
/* [] END OF FILE */
