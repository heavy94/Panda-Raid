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
#include "datalogger.h"
#include "FatFs/diskio.h"
#include "FatFs/ff.h"
#include <stdio.h>

#define FILE_HEADER "\"time\";\"latitude\";\"longitude\";\"speed\";\"course\";\"altitude\";\"geoid\";\"hdop\";\"satellites\"\n"

uint8 logger_active = 0;
FATFS filesystem;
FIL open_file;
//char main_buffer[512] = "caca";

// Private functions declaration
void FatFsError(FRESULT result);

// Public functions definition
//
uint8 dataloggerStart()
{
    uint8 ret_val = 0;
    FRESULT ret_fatfs;
    
    if (CyPins_ReadPin(SD_CD_0) == 0)
    {
        TERM_PutString("SD Card inserted\n");
        
        CyPins_ClearPin(SD_PWR_0);
        SD_SPI_Start();
        // SS signal must be controlled by the SW, thus disconect it from the SPI HW component
        (*(reg32 *)SD_SPI_ss0_m__0__HSIOM) = 
            ((*(reg32 *)SD_SPI_ss0_m__0__HSIOM) & (uint32)~SD_SPI_ss0_m__0__HSIOM_MASK) | (uint32)(SD_SPI_HSIOM_GPIO_SEL << SD_SPI_ss0_m__0__HSIOM_SHIFT);
        SD_SPI_ss0_m_Write(1);
        CyDelay(50);
        
        ret_fatfs = f_mount(&filesystem, "", 1);
        if (ret_fatfs == FR_OK)
        {
            TERM_PutString("SD Card mounted\n");
            
            gps_timestamp_t time = rtcGetTimestamp();
            char filename[15];
            sprintf(filename, "%02u%02u%02u%02u.csv", time.month, time.day, time.hour, time.min); // NO LFN -> 8.3 format
            ret_fatfs = f_open(&open_file, filename, FA_WRITE | FA_CREATE_NEW);
            if (ret_fatfs == FR_OK)
            {
                TERM_PutString("SD File created\n");
                //f_puts(FILE_HEADER, &open_file);
                logger_active = 1;
                ret_val = 1;
            }
            else
            {
                f_mount(NULL, "", 1);
                FatFsError(ret_fatfs);
            }
        }
        else
        {
            FatFsError(ret_fatfs);
        }
    }
    if (ret_val == 0)
    {
        SD_SPI_Stop();
        CyPins_SetPin(SD_PWR_0);
    }
    
    return ret_val;
}

//
void dataloggerStop()
{
    logger_active = 0;
    TERM_PutString("SD File closed. Volume unmounted\n");
    f_close(&open_file);
    f_mount(NULL, "", 1);
    SD_SPI_Stop();
    CyPins_SetPin(SD_PWR_0);
}

//
void dataloggerSaveData(gps_data_t data)
{
    static uint8 count = 0;
    char write_buffer[110] = "";
    /*
    sprintf(write_buffer, "\"%02u:%02u:%02u\";\"%d\260%lu.%05lu'N\";\"%d\260%lu.%05lu'W\";\"%lu\";\"%lu\";\"%lu.%01lu\";\"%lu.%01lu\";\"%lu.%02lu\";\"%u\"",
            data.timestamp.hour, data.timestamp.min, data.timestamp.sec, data.latitude_dg, data.latitude_min/LAT_MIN_DIVIDER, data.latitude_min%LAT_MIN_DIVIDER,
            data.longitude_dg, data.longitude_min/LON_MIN_DIVIDER, data.longitude_min%LON_MIN_DIVIDER, data.speed/SPEED_DIVIDER, data.course/SPEED_DIVIDER, 
            data.altitude/ALTITUDE_DIVIDER, data.altitude%ALTITUDE_DIVIDER, data.geoid/ALTITUDE_DIVIDER, data.geoid%ALTITUDE_DIVIDER, data.hdop/HDOP_DIVIDER, 
            data.hdop%HDOP_DIVIDER, data.n_sat);
    */
    
    char aux_buffer[20];
    // Check valid time data
    if (data.timestamp.hour != INVALID_TIME) 
    {
        sprintf(aux_buffer, "\"%02u:%02u:%02u\";", data.timestamp.hour, data.timestamp.min, data.timestamp.sec);
        strcat(write_buffer, aux_buffer);
    }
    else strcat(write_buffer, "\"\";");
    // Check valid position data
    if (data.latitude_dg != INVALID_POSITION_DG) 
    {
        sprintf(aux_buffer, "\"%d\260%lu.%05lu'N\";", data.latitude_dg, data.latitude_min/LAT_MIN_DIVIDER, data.latitude_min%LAT_MIN_DIVIDER);
        strcat(write_buffer, aux_buffer);
        sprintf(aux_buffer, "\"%d\260%lu.%05lu'W\";", data.longitude_dg, data.longitude_min/LON_MIN_DIVIDER, data.longitude_min%LON_MIN_DIVIDER);
        strcat(write_buffer, aux_buffer);
    }
    else strcat(write_buffer, "\"\";\"\";");
    // Check valid speed data
    if (data.speed != INVALID_SPEED)
    {
        sprintf(aux_buffer, "\"%lu\";", data.speed/SPEED_DIVIDER);
        strcat(write_buffer, aux_buffer);
    }
    else strcat(write_buffer, "\"\";");
    // Check valid course data
    if (data.course != INVALID_COURSE)
    {
        sprintf(aux_buffer, "\"%lu\";", data.course/COURSE_DIVIDER);
        strcat(write_buffer, aux_buffer);
    }
    else strcat(write_buffer, "\"\";");
    // Check valid altitude data
    if (data.altitude != INVALID_SPEED)
    {
        sprintf(aux_buffer, "\"%lu.%01lu\";", data.altitude/ALTITUDE_DIVIDER, data.altitude%ALTITUDE_DIVIDER);
        strcat(write_buffer, aux_buffer);
        sprintf(aux_buffer, "\"%lu.%01lu\";", data.geoid/ALTITUDE_DIVIDER, data.geoid%ALTITUDE_DIVIDER);
        strcat(write_buffer, aux_buffer);
    }
    else strcat(write_buffer, "\"\";\"\";");
    // Check valid HDOP
    if (data.hdop != INVALID_SPEED)
    {
        sprintf(aux_buffer, "\"%lu.%02lu\";", data.hdop/HDOP_DIVIDER, data.hdop%HDOP_DIVIDER);
        strcat(write_buffer, aux_buffer);
    }
    else strcat(write_buffer, "\"\";");
    // Check valid satellite number
    if (data.n_sat != INVALID_SATELLITES)
    {
        sprintf(aux_buffer, "\"%u\"\n", data.n_sat);
        strcat(write_buffer, aux_buffer);
    }
    else strcat(write_buffer, "\"\"\n");
    
    // Write data to file
    f_puts(write_buffer, &open_file);
    /*
    UINT bw = 0;
    f_write(&open_file, main_buffer, 512, &bw);
    f_sync(&open_file);
    */
    
    count++;
    if (count == 10)
    {
        f_sync(&open_file);
        count = 0;
    }
}

uint8 dataloggerGetStatus()
{
    return logger_active;
}

// Private functions definition
void FatFsError(FRESULT result)
{
    switch (result)
    {
        case FR_DISK_ERR:
            TERM_PutString("\n    error: (FR_DISK_ERR) low level error.\n"); break;
            
        case FR_INT_ERR:
            TERM_PutString("\n    error: (FR_INT_ERR)\n"); break; 
            
        case FR_NOT_READY:
            TERM_PutString("\n    error: (FR_NOT_READY) sdcard not ready.\n"); break;
            
        case FR_NO_FILE:
            TERM_PutString("\n    error: (FR_NO_FILE) invalid file.\n"); break;
            
        case FR_NO_PATH:
            TERM_PutString("\n    error: (FR_NO_PATH) invalid path.\n"); break;
            
        case FR_INVALID_NAME:
            TERM_PutString("\n    error: (FR_INVALID_NAME) invalid name.\n"); break;
            
        case FR_DENIED:
            TERM_PutString("\n    error: (FR_DENIED) operation denied.\n"); break;
            
        case FR_EXIST:
            TERM_PutString("\n    error: (FR_EXIST) it exists yet...\n"); break;
            
        case FR_INVALID_OBJECT:
            TERM_PutString("\n    error: (FR_INVALID_OBJECT)\n"); break;
            
        case FR_WRITE_PROTECTED:
            TERM_PutString("\n    error: (FR_WRITE_PROTECTED)\n"); break;
            
        case FR_INVALID_DRIVE:
            TERM_PutString("\n    error: (FR_INVALID_DRIVE)\n"); break;
            
        case FR_NOT_ENABLED:
            TERM_PutString("\n    error: (FR_NOT_ENABLED) sdcard unmounted.\n"); break;
            
        case FR_NO_FILESYSTEM:
            TERM_PutString("\n    error: (FR_NO_FILESYSTEM) no valid FAT volume.\n"); break;  
            
        case FR_MKFS_ABORTED:
            TERM_PutString("\n    error: (FR_MKFS_ABORTED)\n"); break;
            
        case FR_TIMEOUT:
            TERM_PutString("\n    error: (FR_TIMEOUT)\n"); break;
            
        case FR_LOCKED:
            TERM_PutString("\n    error: (FR_LOCKED)\n"); break;
            
        case FR_NOT_ENOUGH_CORE:
            TERM_PutString("\n    error: (FR_NOT_ENOUGH_CORE)\n"); break;     
            
        case FR_TOO_MANY_OPEN_FILES:
            TERM_PutString("\n    error: (FR_TOO_MANY_OPEN_FILES)\n"); break;
            
        case FR_INVALID_PARAMETER:
            TERM_PutString("\n    error: (FR_INVALID_PARAMETER)\n"); break; 
            
        default: {};
    }
}

/* [] END OF FILE */
