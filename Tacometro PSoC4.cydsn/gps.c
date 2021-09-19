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
    
#include "gps.h"
#include "stdlib.h"
   
gps_timestamp_t last_fix_timestamp;
gps_data_t gps_data;

char buf_input[100] = "";
uint8 buf_index = 0;
uint8 cksm_pos = 0;
uint8 computed_cksm = 0; //Checksum is a XOR of all characters between, but not including, '$' and '*'.
uint8 new_sentence = FALSE;
uint8 new_msg_set = FALSE;

//Private functions declaration
uint8 parseSentence();
char* parseTime(char * p_begin);
char* parseDate(char * p_begin);
char* parsePosition(char * p_begin);
char* parseSpeed(char * p_begin);
char* parseCourse(char * p_begin);
char* parseAltitude(char * p_begin);
char* parseHdop(char * p_begin);
char* parseSatellites(char * p_begin);
//void parseFixQuality(char * p_begin);

uint8 from_hex(char a);

//Public functions definition
// Receive incoming data from the gps. 
void gps_receiveData(char c)
{
    switch(c)
    {
        case '$':
        {
            computed_cksm = 0;
            buf_index = 0;
            buf_input[buf_index] = c;
            buf_index++;
            
            break;
        }
        case '*':
        {
            cksm_pos = buf_index;
            buf_input[buf_index] = c;
            buf_index++;
            break;
        }
        case '\r':
        {
            uint8 a = from_hex(buf_input[cksm_pos+1]);
            uint8 b = from_hex(buf_input[cksm_pos+2]);
            uint8 received_cksm = 16 * a + b;
            
            for(uint8 i = 1; i < cksm_pos; i++)
            {
                computed_cksm ^= buf_input[i];
            }
            
            if(received_cksm == computed_cksm)
            {
                new_msg_set = parseSentence();
            }
            
            break;
        }
        case '\n':
        {
            break;
        }
        default:
        {
            buf_input[buf_index] = c;
            buf_index++;
            //computed_cksm ^= c;
            
            break;
        }
    }
}

void gps_getData(volatile gps_data_t * data)
{
    *data = gps_data;
}

uint8 gps_getQuality()
{
    uint8 ret_val = GPS_QUALITY_BAD;
    
    if ((gps_data.n_sat >= 4) && (gps_data.altitude != INVALID_ALTITUDE))
    {
        ret_val = GPS_QUALITY_POOR;
        if (gps_data.n_sat >= 8)
        {
            ret_val = GPS_QUALITY_GOOD;
        }
    }
    
    return ret_val;
}

// Returns TRUE if a neew full set of available NMEA messages (with the same Fix time) has been decoded.
uint8 gps_newDataAvailable()
{
    uint8 ret_val = FALSE;
    
    if (new_msg_set)
    {
        ret_val = TRUE;
        new_msg_set = FALSE;
    }
    
    return ret_val;
}

//Private functions definitions
// Parse a valid NMEA sentence. Call only when a valid checksum sentence has been received.
// Returns TRUE when a full set of available NMEA messages (with the same Fix time) has been decoded.
uint8 parseSentence()
{
    static uint8 new_gga_sec = 0;
    static uint8 new_gga_scen = 0;
    static uint8 new_rmc_sec = 0;
    static uint8 new_rmc_scen = 0;
    uint8 new_data = FALSE;
    
    //GGA (Global Positioning System Fix Data)
        //UTC Time
        //Latitude and N/S indicator
        //Longitude and W/E indicator
        //Fix quality
        //N satellites tracked *
        //HDP *
        //Altitude MSL and units *
        //Height of geoid and units *
        //(empty field)
        //(empty field)
    if (strstr(buf_input, NMEA_GGA) != NULL)
    {
        char *p_buf;
        
        p_buf = strpbrk(buf_input, ","); // time
        p_buf = parseTime(p_buf);
        p_buf += 1; // lat
        p_buf = strpbrk(p_buf, ",") + 1; // NS
        p_buf = strpbrk(p_buf, ",") + 1; // long
        p_buf = strpbrk(p_buf, ",") + 1; // EW
        p_buf = strpbrk(p_buf, ",") + 1; // quality
        p_buf = strpbrk(p_buf, ",");     // numSV
        p_buf = parseSatellites(p_buf);
        p_buf = parseHdop(p_buf);
        p_buf = parseAltitude(p_buf);
        
        new_gga_sec = gps_data.timestamp.sec;
        new_gga_scen = gps_data.timestamp.sec_cent;
    }
    
    //RMC (Recommended Minimum Data)
        //UTC Time *
        //Status
        //Latitude and N/S indicator *
        //Longitude and W/E indicator *
        //Speed (knots) *
        //Course (degrees) *
        //UTC Date *
        //(empty field)
        //(empty field)
        //Mode indicator
    if (strstr(buf_input, NMEA_RMC) != NULL)
    {
        char *p_buf;
        
        p_buf = strpbrk(buf_input, ","); // time
        p_buf = parseTime(p_buf);
        p_buf = strpbrk(p_buf + 1, ","); // status
        p_buf = parsePosition(p_buf);
        p_buf = parseSpeed(p_buf);
        p_buf = parseCourse(p_buf);
        p_buf = parseDate(p_buf);
        
        new_rmc_sec = gps_data.timestamp.sec;
        new_rmc_scen = gps_data.timestamp.sec_cent;
    }
    
    if ((new_gga_sec == new_rmc_sec) && (new_gga_scen == new_rmc_scen) 
        && (gps_data.timestamp.hour != INVALID_TIME))
    {
        new_data = TRUE;
    }
    
    return new_data;
}

//
char* parseTime(char * p_begin) //hhmmss.ss
{
    p_begin++;
    
    if ((*p_begin) != ',')
    {
        gps_data.timestamp.hour = (from_hex(*p_begin) * 10) + from_hex(*(p_begin + 1));
        gps_data.timestamp.min = (from_hex(*(p_begin + 2)) * 10) + from_hex(*(p_begin + 3));
        gps_data.timestamp.sec = (from_hex(*(p_begin + 4)) * 10) + from_hex(*(p_begin + 5));
        gps_data.timestamp.sec_cent = (from_hex(*(p_begin + 7)) * 10) + from_hex(*(p_begin + 8));
    }
    else
    {
        gps_data.timestamp.hour = INVALID_TIME;
    }
    
    p_begin = strpbrk(p_begin, ",");
    return p_begin;
}

char* parseDate(char * p_begin) //ddmmyy
{
    p_begin++;
    
    if ((*p_begin) != ',')
    {
        gps_data.timestamp.day = (from_hex(*p_begin) * 10) + from_hex(*(p_begin + 1));
        gps_data.timestamp.month = (from_hex(*(p_begin + 2)) * 10) + from_hex(*(p_begin + 3));
        gps_data.timestamp.year = (from_hex(*(p_begin + 4)) * 10) + from_hex(*(p_begin + 5));
    }
    else
    {
        gps_data.timestamp.year = INVALID_DATE;
    }
    
    p_begin = strpbrk(p_begin, ",");
    return p_begin;
}

char* parsePosition(char * p_begin) //ddmm.mmmmm,N,dddmm.mmmmm,W
{
    p_begin++;
    
    if ((*p_begin) != ',')
    {
        gps_data.latitude_dg = (from_hex(*p_begin) * 10) + from_hex(*(p_begin + 1));
        gps_data.latitude_min = (from_hex(*(p_begin + 2)) * 1000000) + (from_hex(*(p_begin + 3)) * 100000) +
                                (from_hex(*(p_begin + 5)) * 10000) + (from_hex(*(p_begin + 6)) * 1000) + 
                                (from_hex(*(p_begin + 7)) * 100) + (from_hex(*(p_begin + 8)) * 10) + from_hex(*(p_begin + 9));
        if(*(p_begin + 11) == 'S')
        {
            gps_data.latitude_dg = -gps_data.latitude_dg;
        }
        gps_data.longitude_dg = (from_hex(*(p_begin + 13)) * 100) + (from_hex(*(p_begin + 14)) * 10) + from_hex(*(p_begin + 15));
        gps_data.longitude_min = (from_hex(*(p_begin + 16)) * 1000000) + (from_hex(*(p_begin + 17)) * 100000) + 
                                 (from_hex(*(p_begin + 19)) * 10000) + (from_hex(*(p_begin + 20)) * 1000) + 
                                 (from_hex(*(p_begin + 21)) * 100) + (from_hex(*(p_begin + 22)) * 10) + from_hex(*(p_begin + 23));
        if(*(p_begin + 25) == 'E')
        {
            gps_data.longitude_dg = -gps_data.longitude_dg;
        }
    }
    else
    {
        gps_data.latitude_dg = INVALID_POSITION_DG;
        gps_data.longitude_dg = INVALID_POSITION_DG;
    }
    
    p_begin += 26;
    return p_begin;
}

char* parseSpeed(char * p_begin)
{
    p_begin++;
    
    if ((*p_begin) != ',')
    {
        uint32 speed_knots = (uint32)atoi(p_begin);
        p_begin = strpbrk(p_begin + 1, ".") + 1;
        speed_knots = speed_knots * 1000 + (uint32)atoi(p_begin);
        gps_data.speed = KNOTS_TO_KMH(speed_knots);
    }
    else
    {
        gps_data.speed = INVALID_SPEED;
    }
    
    p_begin = strpbrk(p_begin, ",");
    return p_begin;
}

char* parseCourse(char * p_begin)
{
    p_begin++;
    
    if ((*p_begin) != ',')
    {
        uint32 course = (uint32)atoi(p_begin);
        p_begin = strpbrk(p_begin + 1, ".") + 1;
        gps_data.course = course * 100 + (uint32)atoi(p_begin);
    }
    else
    {
        gps_data.course = INVALID_COURSE;
    }
    
    p_begin = strpbrk(p_begin, ",");
    return p_begin;
}

char* parseAltitude(char * p_begin)
{
    p_begin++;
    
    if ((*p_begin) != ',')
    {
        // Altitude
        uint32 altitude = (uint32)atoi(p_begin);
        p_begin = strpbrk(p_begin + 1, ".") + 1;
        gps_data.altitude = altitude * 10 + (uint32)atoi(p_begin);
        p_begin = strpbrk(p_begin + 1, ",") + 1; // Altitude units (always meters)
        // Geoid separation
        p_begin = strpbrk(p_begin + 1, ",") + 1;
        uint32 geoid = (uint32)atoi(p_begin);
        p_begin = strpbrk(p_begin + 1, ".") + 1;
        gps_data.geoid = geoid * 10 + (uint32)atoi(p_begin);
        p_begin = strpbrk(p_begin + 1, ",") + 1; // Altitude units (always meters)
    }
    else
    {
        gps_data.altitude = INVALID_ALTITUDE;
        p_begin = strpbrk(p_begin, ",") + 1;
        p_begin = strpbrk(p_begin, ",") + 1;
        p_begin = strpbrk(p_begin, ",") + 1;
    }
    
    p_begin = strpbrk(p_begin, ",");
    return p_begin;
}

char* parseHdop(char * p_begin)
{
    p_begin++;
    
    if ((*p_begin) != ',')
    {
        uint32 hdop = (uint32)atoi(p_begin);
        p_begin = strpbrk(p_begin + 1, ".") + 1;
        gps_data.hdop = hdop * 100 + (uint32)atoi(p_begin);
    }
    else
    {
        gps_data.hdop = INVALID_HDOP;
    }
    
    p_begin = strpbrk(p_begin + 1, ",");
    return p_begin;
}

char* parseSatellites(char * p_begin)
{
    p_begin++;
    
    gps_data.n_sat = (uint8)atoi(p_begin);
    
    p_begin = strpbrk(p_begin + 1, ",");
    return p_begin;
}

uint8 from_hex(char a) 
{
    if(a >= 'A' && a <= 'F')
    {
        return a - 'A' + 10;
    }
    else if(a >= 'a' && a <= 'f')
    {
        return a - 'a' + 10;
    }
    else
    {
        return a - '0';
    }
}

/* [] END OF FILE */
