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

#define KNOTS_TO_KMH(x) ((x * 1852) / 1000)
    
//NMEA sentence types
#define NMEA_GGA "GPGGA"
#define NMEA_RMC "GPRMC"

typedef struct
{
    uint8 year;
    uint8 month;
    uint8 day;
    uint8 hour;
    uint8 min;
    uint8 sec;
    uint8 sec_cent;
} gps_timestamp_t;

typedef struct
{
    gps_timestamp_t timestamp;          //RMC
    int16   latitude_dg;                //RMC
    uint32  latitude_min;   // x100000  //RMC
    int16   longitude_dg;               //RMC
    uint32  longitude_min;  // x100000  //RMC
    uint32  speed;          //          //RMC
    uint32  course;         //          //RMC
    uint32  altitude;       // x10      //GGA
    uint32  geoid;          // x10      //GGA
    uint32  hdop;           // x100     //GGA
    uint8   n_sat;                      //GGA
    uint8   fix_status;                 //GGA
} gps_data_t;
    

//Public functions
uint8 gps_receiveData(char c);
uint8 gps_getData(gps_data_t * data);

#endif
/* [] END OF FILE */
