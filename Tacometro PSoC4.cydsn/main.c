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
#include "display.h"
#include <stdlib.h>
#include <stdio.h>

//#define MILLIS (0xFFFFFFFFu - Timer_Millis_ReadCounter())
#define TRUE 1
#define FALSE 0
#define MILLIS 0;
#define CR 0x0D //ASCII for Carriage Return

gps_data_t nav_data;

volatile uint8 reset_rpm_count = FALSE;
void read_rpm_in(uint32 * rpm_count);
uint32 update_rpm(uint32 rpm_count);
CY_ISR_PROTO(ISR_rpm_counter_tc);

int main(void)
{
    uint32 rpm_count = 0;
    
    uint32 speed = 0;
    uint32 heading = 0;
    uint32 rpm = 0;
    uint8 status = 0;
    uint8 dir = 0;
    uint16 bright = 50;
    uint8 btn_zero = FALSE;
    uint8 btn_one = FALSE;
    uint8 prev_btn_zero = FALSE;
    uint8 prev_btn_one = FALSE;
    
    uint8_t poll_port_config[9] = {0xB5, 0x62, 0x06, 0x00, 0x01, 0x00, 0x01, 0x08, 0x22};
    uint8_t port_config[28] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00,
                               0x00, 0x96, 0x00, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x93, 0x90}; // Sets baud rate to 38400
    
    uint8_t no_gll[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A}; // No GLL
    uint8_t no_gsa[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x31}; // No GSA
    uint8_t no_gsv[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x38}; // No GSV
    uint8_t no_vtg[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x46}; // No VTG
    
    uint8_t fix_rate[14] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x64, 0x00, 0x01, 0x00, 0x01, 0x00, 0x7A, 0x12}; // Sets the update rate to 10Hz
    
    //TODO configuracion SBAS
    
    CyGlobalIntEnable;
    
    isr_taco_StartEx(ISR_rpm_counter_tc);
    
    TERM_Start();
    //CLK_UART_GNSS_Start();
    GNSS_Start();
    CyDelay(200);
    GNSS_SpiUartPutArray(no_gll, 16);
    CyDelay(200);
    GNSS_SpiUartPutArray(no_gsa, 16);
    CyDelay(200);
    GNSS_SpiUartPutArray(no_gsv, 16);
    CyDelay(200);
    GNSS_SpiUartPutArray(no_vtg, 16);
    CyDelay(200);
    
    
    PWM_Bright_Start();
    PWM_Bright_WriteCompare(bright);
    display_init();
    Timer_RPM_Start();
    
    TERM_PutString("INICIO\n\n");
    /*
    GNSS_SpiUartPutArray(port_config, 28);
    CyDelay(200);
    CLK_UART_GNSS_SetDividerValue(52);
    CyDelay(200);
    GNSS_SpiUartPutArray(fix_rate, 14);
    */
    for(;;)
    {
        /*
        {
            if (status >= 3)
            {
                status = 0;
            }
            else
            {
                status++;
            }
            if (speed < 999) 
            {
                speed++;
            }
            else 
            {  
                speed = 0;
            }
            if (heading == 0)
            {
                heading = 999;
            }
            else
            {
                heading--;
            }
            if (dir == 0)
            {
                if (rpm < 5999)
                {
                    rpm += 250;
                }
                else
                {
                    dir = 1;
                    rpm -= 250;
                }
            }
            else
            {
                if (rpm == 0)
                {
                    dir = 0;
                    rpm += 250;
                }
                else
                {
                    rpm -= 250;
                }
            }
        }
        */
        // Update RPM value
        read_rpm_in(&rpm_count);
        if (reset_rpm_count)
        {
            rpm = update_rpm(rpm_count);
            reset_rpm_count = FALSE;
        }
        //
        
        // Buttons reading
        btn_zero = CyPins_ReadPin(BTN_0);
        if ((btn_zero != prev_btn_zero) && (btn_zero == FALSE))
        {
            bright += 25;
            if (bright > 100) bright = 0;
        }
        prev_btn_zero = btn_zero;
        //
        
        PWM_Bright_WriteCompare(bright);
        //display_update(speed, heading, rpm, status);
        //CyDelay(200);
        
        while(GNSS_SpiUartGetRxBufferSize() > 0)
        {
            char c = GNSS_UartGetChar();
            TERM_PutChar(c);
            gps_receiveData(c);
            /*
            if(gps_receiveData(c) != 0)
            {
                gps_getTime(&hora, &min, &seg, &seg_d);
                char aux[30];
                sprintf(aux, "\n%02d:%02d:%02d.%02d\n", hora, min, seg, seg_d);
                TERM_PutString(aux);
                gps_getPosition(&lat_dg, &lat_min, &lat_min_d, &lon_dg, &lon_min, &lon_min_d);
                sprintf(aux, "%d\260%d.%ld'N  %d\260%d.%ld'W\n", lat_dg, lat_min, lat_min_d, lon_dg, lon_min, lon_min_d);
                TERM_PutString(aux);
            }
            */
        }
        
        if (gps_getData(&nav_data))
        {
            char aux[50];
            sprintf(aux, "\n%02d:%02d:%02d - %02d/%02d/%02d\n", nav_data.timestamp.hour, nav_data.timestamp.min, nav_data.timestamp.sec,
                                                                nav_data.timestamp.day, nav_data.timestamp.month, nav_data.timestamp.year);
            TERM_PutString(aux);
            sprintf(aux, "%d\260%lu.%05lu'N %d\260%lu.%05lu'W\n", nav_data.latitude_dg, nav_data.latitude_min/100000, nav_data.latitude_min%100000,
                                                              nav_data.longitude_dg, nav_data.longitude_min/100000, nav_data.longitude_min%100000);
            TERM_PutString(aux);
            uint32 speed_kmh =  KNOTS_TO_KMH(nav_data.speed);
            sprintf(aux, "%lu.%03luknots - %lu.%03luKm/h\n", nav_data.speed/1000, nav_data.speed%1000, speed_kmh/1000, speed_kmh%1000);
            TERM_PutString(aux);
            sprintf(aux, "Course: %lu.%02lu\260\n", nav_data.course/100, nav_data.course%100);
            TERM_PutString(aux);
            
            display_update(speed_kmh%1000, nav_data.course/100, rpm, status);
        }
        
    }
}

void read_rpm_in(uint32 * rpm_count)
{
    static uint8 prev_val = TRUE;   // Pull-up input
    uint8 curr_val = CyPins_ReadPin(RPM_IN_0);
    
    if ((curr_val != prev_val) && (curr_val == FALSE))
    {
        rpm_count++;
    }
    prev_val = curr_val;
}

uint32 update_rpm(uint32 rpm_count)
{
    /* NOTE: Need to define the relation between alternator and engine*/
    // 'rpm_count' contains the number activations registered on the
    // pin 'RPM_IN' in 0.1 seconds. Multiply by 600 to obtain the 
    // value for minutes (RPM). But each revolution of the alternator
    // register 6 activations so divide by 6. Final multiplication 
    // factor is 100.
    uint32 val = rpm_count * 100;
    rpm_count = 0;
    
    return val;
}

CY_ISR(ISR_rpm_counter_tc)
{
    Timer_RPM_ClearInterrupt(Timer_RPM_INTR_MASK_TC);
    reset_rpm_count = TRUE;    
}

/* [] END OF FILE */