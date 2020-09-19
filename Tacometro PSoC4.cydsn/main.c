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
#include "FatFs\diskio.h"
#include "FatFs\ff.h"
#include "gps.h"
#include <stdlib.h>
#include <stdio.h>

//#define MILLIS (0xFFFFFFFFu - Timer_Millis_ReadCounter())
#define TRUE 1
#define FALSE 0
#define MILLIS 0;
#define CR 0x0D //ASCII for Carriage Return

uint8 hora = 0;
uint8 min = 0;
uint8 seg = 0;
uint8 seg_d = 0;

void Test_A();
void Test_B();
void FatFsError(FRESULT result);


int main(void)
{
    CyGlobalIntEnable;
    
    TERM_Start();
    GNSS_Start();
    
    TERM_PutString("INICIO\n\n");
    
    for(;;)
    {
        while(GNSS_SpiUartGetRxBufferSize() > 0)
        {
            char c = GNSS_UartGetChar();
            TERM_PutChar(c);
            
            if(gps_receiveData(c) != 0)
            {
                gps_getTime(&hora, &min, &seg, &seg_d);
                char aux[30];
                sprintf(aux, "\n%02d:%02d:%02d.%02d\n", hora, min, seg, seg_d);
                TERM_PutString(aux);
            }
        }
    }
}
/*
void Test_B()
{
    FATFS fatFs;
    FIL pfile;
    uint8 resultF;
    
    resultF = f_mount(&fatFs, "", 1);
    
    if (resultF == RES_OK)
    {
        resultF = f_open(&pfile, "testbin.txt", FA_WRITE | FA_CREATE_ALWAYS);
        
        if (resultF == FR_OK)
        {
            TERM_UartPutString("Archivo creado.\nIniciando prueba de rendimiento en modo binario.\n");
            TERM_UartPutString("Fase 1. 2000 escrituras individuales de int32\n");
            
            uint32 init_time = MILLIS;
            for (uint16 i = 0; i < 2000; i++)
            {
                int32 num = random();
                unsigned int n_write = 0;
                //FS_FWrite(&num, sizeof(num), 1, pfile);
                resultF = f_write(&pfile, &num, 4, &n_write);
                FatFsError(resultF);
            }
            uint32 end_time = MILLIS;
            
            uint32 total_time = end_time - init_time;
            uint32 mean_time = (end_time - init_time)/2000;
            char aux[100];
            sprintf(aux, "Fin Fase 1.\nTiempo total de escritura: %lu ms\nTiempo medio de cada escritura: %lu ms\n", total_time, mean_time);
            TERM_UartPutString(aux);
            
            
            TERM_UartPutString("Fase 2. 10 escrituras 200 int32\n");
            int32 num_buffer[200];
            init_time = MILLIS;
            for (uint8 i = 0; i < 10; i++)
            {
                for (uint8 i = 0; i < 200; i++)
                {
                    num_buffer[i] = random();
                }
                unsigned int n_write = 0;
                //FS_FWrite(num_buffer, sizeof(int32), 200, pfile);
                resultF = f_write(&pfile, num_buffer, 4*200, &n_write);
                FatFsError(resultF);
            }
            end_time = MILLIS;
            
            total_time = end_time - init_time;
            mean_time = (end_time - init_time)/10;
            sprintf(aux, "Fin Fase 2.\nTiempo total de escritura: %lu ms\nTiempo medio de cada escritura: %lu ms\n", total_time, mean_time);
            TERM_UartPutString(aux);
            
            if (FR_OK == f_close(&pfile))
            {
                TERM_UartPutString("Archivo cerrado\n");
            }
            else
            {
                FatFsError(resultF);
            }
            
        }
        else
        {
            FatFsError(resultF);
        }
    }
    else
    {
        FatFsError(resultF);
    }
}

void FatFsError(FRESULT result)
{
    
    switch (result)
    {
        case FR_DISK_ERR:
            TERM_UartPutString("\n    error: (FR_DISK_ERR) low level error.\n"); break;
            
        case FR_INT_ERR:
            TERM_UartPutString("\n    error: (FR_INT_ERR)\n"); break; 
            
        case FR_NOT_READY:
            TERM_UartPutString("\n    error: (FR_NOT_READY) sdcard not ready.\n"); break;
            
        case FR_NO_FILE:
            TERM_UartPutString("\n    error: (FR_NO_FILE) invalid file.\n"); break;
            
        case FR_NO_PATH:
            TERM_UartPutString("\n    error: (FR_NO_PATH) invalid path.\n"); break;
            
        case FR_INVALID_NAME:
            TERM_UartPutString("\n    error: (FR_INVALID_NAME) invalid name.\n"); break;
            
        case FR_DENIED:
            TERM_UartPutString("\n    error: (FR_DENIED) operation denied.\n"); break;
            
        case FR_EXIST:
            TERM_UartPutString("\n    error: (FR_EXIST) it exists yet...\n"); break;
            
        case FR_INVALID_OBJECT:
            TERM_UartPutString("\n    error: (FR_INVALID_OBJECT)\n"); break;
            
        case FR_WRITE_PROTECTED:
            TERM_UartPutString("\n    error: (FR_WRITE_PROTECTED)\n"); break;
            
        case FR_INVALID_DRIVE:
            TERM_UartPutString("\n    error: (FR_INVALID_DRIVE)\n"); break;
            
        case FR_NOT_ENABLED:
            TERM_UartPutString("\n    error: (FR_NOT_ENABLED) sdcard unmounted.\n"); break;
            
        case FR_NO_FILESYSTEM:
            TERM_UartPutString("\n    error: (FR_NO_FILESYSTEM) no valid FAT volume.\n"); break;  
            
        case FR_MKFS_ABORTED:
            TERM_UartPutString("\n    error: (FR_MKFS_ABORTED)\n"); break;
            
        case FR_TIMEOUT:
            TERM_UartPutString("\n    error: (FR_TIMEOUT)\n"); break;
            
        case FR_LOCKED:
            TERM_UartPutString("\n    error: (FR_LOCKED)\n"); break;
            
        case FR_NOT_ENOUGH_CORE:
            TERM_UartPutString("\n    error: (FR_NOT_ENOUGH_CORE)\n"); break;     
            
        case FR_TOO_MANY_OPEN_FILES:
            TERM_UartPutString("\n    error: (FR_TOO_MANY_OPEN_FILES)\n"); break;
            
        case FR_INVALID_PARAMETER:
            TERM_UartPutString("\n    error: (FR_INVALID_PARAMETER)\n"); break; 
            
        default: {}; break;
    }
}
*/
/* [] END OF FILE */

