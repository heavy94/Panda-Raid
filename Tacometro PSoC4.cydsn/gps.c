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
uint8 new_data = 0;

char buf_input[100] = "";
uint8 buf_index = 0;
uint8 cksm_pos = 0;
uint8 computed_cksm = 0; //Checksum is a XOR of all characters between, but not including, '$' and '*'.
uint8 new_sentence = FALSE;

//Private functions declaration
uint8 parseSentence();
void parseTime(char * p_begin);
void parseDate(char * p_begin);
void parsePosition(char * p_begin);
void parseSpeed(char * p_begin);
void parseCourse(char * p_begin);
void parseAltitude(char * p_begin);
void parseHdop(char * p_begin);
void parseSatellites(char * p_begin);
void parseFixQuality(char * p_begin);

uint8 from_hex(char a);

//Public functions definition
//Receive incoming data from the gps. Returns TRUE if the recieved NMEA sentence checksum is valid.
uint8 gps_receiveData(char c)
{
    uint8 valid_sentence = FALSE;
    
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
                valid_sentence = parseSentence();
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
    
    return valid_sentence;
}
uint8 gps_getData(gps_data_t * data)
{
    uint8 ret_val = FALSE;
    
    if (new_data)
    {
        *data = gps_data;
        new_data = 0;
        ret_val = TRUE;
    }
    
    return ret_val;
}

//Private functions definitions
//Parse a valid NMEA sentence. Call only when a valid checksum sentence has been received.
uint8 parseSentence()
{
    //GGA (Global Positioning System Fix Data)
        //UTC Fix time
        //Latitude and N/S indicator
        //Longitude and W/E indeicator
        //Fix quality
        //N satellites tracked
        //HDP
        //Altitude MSL (m)
        //Height of geoid (m)
        //(empty field)
        //(empty field)
    /*
    if (strstr(buf_input, NMEA_GGA) != NULL)
    {
        char *p_buf;
        
        p_buf = strtok(buf_input, ",");
        p_buf = strtok(NULL, ",");
        parseTime(p_buf);
        p_buf = strtok(NULL, ",");
        parsePosition(p_buf);
        return TRUE;
    }*/
    
    if (strstr(buf_input, NMEA_RMC) != NULL)
    {
        char *p_buf;
        p_buf = strpbrk(buf_input, ",") + 1;
        parseTime(p_buf);
        p_buf = strpbrk(p_buf, ",") + 1;
        p_buf = strpbrk(p_buf, ",") + 1;
        parsePosition(p_buf);
        p_buf = strpbrk(p_buf, ",") + 1;
        p_buf = strpbrk(p_buf, ",") + 1;
        p_buf = strpbrk(p_buf, ",") + 1;
        p_buf = strpbrk(p_buf, ",") + 1;
        parseSpeed(p_buf);
        p_buf = strpbrk(p_buf, ",") + 1;
        parseCourse(p_buf);
        p_buf = strpbrk(p_buf, ",") + 1;
        parseDate(p_buf);
        new_data = 1;
    }
    
    return FALSE;
}

//
void parseTime(char * p_begin) //hhmmss.ss
{
    gps_data.timestamp.hour = (from_hex(*p_begin) * 10) + from_hex(*(p_begin + 1));
    gps_data.timestamp.min = (from_hex(*(p_begin + 2)) * 10) + from_hex(*(p_begin + 3));
    gps_data.timestamp.sec = (from_hex(*(p_begin + 4)) * 10) + from_hex(*(p_begin + 5));
    gps_data.timestamp.sec_cent = (from_hex(*(p_begin + 7)) * 10) + from_hex(*(p_begin + 8));
}

void parseDate(char * p_begin) //ddmmyy
{
    gps_data.timestamp.day = (from_hex(*p_begin) * 10) + from_hex(*(p_begin + 1));
    gps_data.timestamp.month = (from_hex(*(p_begin + 2)) * 10) + from_hex(*(p_begin + 3));
    gps_data.timestamp.year = (from_hex(*(p_begin + 4)) * 10) + from_hex(*(p_begin + 5));
}

void parsePosition(char * p_begin) //ddmm.mmmmm,N,dddmm.mmmmm,W
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

void parseSpeed(char * p_begin)
{
    uint32 speed_knots = (uint32)atoi(p_begin);
    p_begin = strpbrk(p_begin + 1, ".") + 1;
    speed_knots = speed_knots * 1000 + (uint32)atoi(p_begin);
    gps_data.speed = speed_knots;
}

void parseCourse(char * p_begin)
{
    uint32 course = (uint32)atoi(p_begin);
    p_begin = strpbrk(p_begin + 1, ".") + 1;
    gps_data.course = course * 100 + (uint32)atoi(p_begin);
}

void parseAltitude(char * p_begin);
void parseHdop(char * p_begin);
void parseSatellites(char * p_begin);
void parseFixQuality(char * p_begin);

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
