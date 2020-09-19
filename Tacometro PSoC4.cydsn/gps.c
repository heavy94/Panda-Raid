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
   
//Fix data
typedef struct
{
    uint8 hour;
    uint8 min;
    uint8 sec;
    uint8 sec_d;
} fix_time;
    
struct
{
    int16 lat_dg;
    int16 lat_min;
    int32 lat_min_d;
    int16 lon_dg;
    int16 lon_min;
    int32 lon_min_d;
} fix_position;

fix_time last_fix;

char buf_input[100] = "";
uint8 buf_index = 0;
uint8 cksm_pos = 0;
uint8 computed_cksm = 0; //Checksum is a XOR of all characters between, but not including, '$' and '*'.
uint8 new_sentence = FALSE;

//Private functions declaration
uint8 parseSentence();
void parseTime(char  * p_begin);
void parsePosition(char  * p_begin);
void parseFixQuality();
void parseSatellites();
void parseHdp();
void parseAltitude();

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

void gps_getTime(uint8 * hour, uint8 * min, uint8 * sec, uint8 * sec_d)
{
    *hour = last_fix.hour;
    *min = last_fix.min;
    *sec = last_fix.sec;
    *sec_d = last_fix.sec_d;
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
    if (strstr(buf_input, NMEA_GGA) != NULL)
    {
        char *p_buf;
        
        p_buf = strchr(buf_input, ',') + 1;
        parseTime(p_buf);
        p_buf = strchr(buf_input, ',') + 1;
        parsePosition(p_buf);
        return TRUE;
    }
    
    return FALSE;
}

//
void parseTime(char * p_begin) //hhmmss.ss
{
    last_fix.hour = (from_hex((*p_begin)) * 10) + from_hex(*(p_begin + 1));
    last_fix.min = (from_hex(*(p_begin + 2)) * 10) + from_hex(*(p_begin + 3));
    last_fix.sec = (from_hex(*(p_begin + 4)) * 10) + from_hex(*(p_begin + 5));
    last_fix.sec_d = (from_hex(*(p_begin + 7)) * 10) + from_hex(*(p_begin + 8));
}
void parsePosition(char * p_begin) //ddmm.mmmmm,N,dddmm.mmmmm,W
{
    fix_position.lat_dg = ((*p_begin) * 10) + *(p_begin + 1);
    fix_position.lat_min = (*(p_begin + 2) * 10) + *(p_begin + 3);
    fix_position.lat_min_d = (*(p_begin + 5) * 10000) + (*(p_begin + 6) * 1000) + (*(p_begin + 7) * 100) + (*(p_begin + 8) * 10) + *(p_begin + 9);
    if(*(p_begin + 11) == 'S')
    {
        fix_position.lat_dg = -fix_position.lat_dg;
    }
    fix_position.lon_dg = (*(p_begin + 13) * 100) + (*(p_begin + 14) * 10) + *(p_begin + 15);
    fix_position.lon_min = (*(p_begin + 16) * 10) + *(p_begin + 17);
    fix_position.lon_min_d = (*(p_begin + 19) * 10000) + (*(p_begin + 20) * 1000) + (*(p_begin + 21) * 100) + (*(p_begin + 22) * 10) + *(p_begin + 23);
    if(*(p_begin + 25) == 'E')
    {
        fix_position.lon_dg = -fix_position.lon_dg;
    }
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
