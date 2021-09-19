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
#include "datalogger.h"
#include "display.h"
#include "buttons.h"
#include <stdlib.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0
#define CR 0x0D //ASCII for Carriage Return
#define REFRESH_RATE_BUTTONS 100 //ms

volatile gps_data_t nav_data;
uint8 prev_timestamp_sec;

// GNSS receive and parse
CY_ISR_PROTO(ISR_gnss_new_byte);

// RPM counter
CY_ISR_PROTO(ISR_rpm_counter_tc);
uint32 update_rpm();

// Millis counter
volatile uint64 millis_count = 0;
CY_ISR_PROTO(ISR_millis);
uint64 getMillis();

int main(void)
{
    uint32 rpm_count = 0;
    uint32 rpm = 0;
    
    uint8 status = 0;
    uint16 bright = 60;
    
    uint64 prev_millis_buttons = 0;
    
    const uint8 poll_port_config[9] = {0xB5, 0x62, 0x06, 0x00, 0x01, 0x00, 0x01, 0x08, 0x22};
    const uint8 port_config[28] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00,
                                   0x00, 0x96, 0x00, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x93, 0x90}; // Sets baud rate to 38400   
    const uint8 no_gll[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A}; // No GLL
    const uint8 no_gsa[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x31}; // No GSA
    const uint8 no_gsv[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x38}; // No GSV
    const uint8 no_vtg[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x46}; // No VTG
    const uint8 fix_rate[14] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x64, 0x00, 0x01, 0x00, 0x01, 0x00, 0x7A, 0x12}; // Sets the update rate to 10Hz
    
    //TODO configuracion SBAS
    
    CyGlobalIntEnable;
    
    TERM_Start();
    TERM_PutString("INICIO\n\n");
    
    // Start and configure GNSS receiver
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
    GNSS_SpiUartPutArray(port_config, 28);
    CyDelay(200);
    CLK_UART_GNSS_SetDividerValue(52);
    CyDelay(200);
    GNSS_SpiUartPutArray(fix_rate, 14);
    // Other system initialization
    PWM_Bright_Start();
    PWM_Bright_WriteCompare(bright);
    display_init();
    display_test();
    CyDelay(2000);
    initButtons(REFRESH_RATE_BUTTONS);
    // Enable tachometer interrupt
    Timer_RPM_Start();
    isr_taco_StartEx(ISR_rpm_counter_tc);
    ISR_millis_StartEx(ISR_millis);
    // Enable GNSS UART interrupt to parse data
    GNSS_SpiUartClearRxBuffer();
    ISR_gnss_rx_StartEx(ISR_gnss_new_byte);
    
    for(;;)
    {        
        // Buttons reading       
        if ((getMillis() - prev_millis_buttons) >= REFRESH_RATE_BUTTONS)
        {
            updateButtons();
            prev_millis_buttons = getMillis();
        
            if (getButtonShort_0()) // Update display brightness
            {
                bright += 40;
                if (bright > 100) bright = 20;
                PWM_Bright_WriteCompare(bright);
            }
            if (getButtonLong_1()) // Start/Stop datalogger
            {
                if (dataloggerGetStatus() == 0)
                {
                    if (gps_getQuality() != 0)
                    {
                        TERM_PutString("Start datalogger\n");
                        if (dataloggerStart()) TERM_PutString("Datalogger ON\n");
                        else TERM_PutString("Couldn't start datalogger\n");
                        prev_timestamp_sec = nav_data.timestamp.sec;
                    }
                    else TERM_PutString("Not enough signal quality to start datalogger\n");
                }
                else
                {
                    TERM_PutString("Stop datalogger\n");
                    dataloggerStop();
                }
            }
        }
        
        // Data saving
        if ((dataloggerGetStatus() == 1) && (nav_data.timestamp.sec != prev_timestamp_sec))
        {
            prev_timestamp_sec = nav_data.timestamp.sec;
            uint64 interval = getMillis();
            dataloggerSaveData(nav_data);
            interval = getMillis() - interval;
            char buff[20];
            sprintf(buff, "SAVE -> %4lums -> ", (uint32)interval);
            TERM_PutString(buff);
        }       
    }
}

uint32 update_rpm()
{
    uint32 ret_val;
    /* NOTE: Relation between engine RPM and alternator phase RPM is '6'*/
    /* Timer_RPM counts the duration between 
    
    */
    uint32 aux_val = Timer_RPM_ReadCaptureBuf();
    if (aux_val > 10000)
    {
        //TERM_PutString("timer stoped\n");
        ret_val = 0;
    }
    else
    {
        ret_val = 1000000 / aux_val;
        //char buf[20];
        //sprintf(buf, "aux_val: %lu\n", ret_val);
        //TERM_PutString(buf);
    }
    return ret_val;
}

uint64 getMillis()
{
    return millis_count;
}

CY_ISR(ISR_gnss_new_byte)
{
    char c = GNSS_UartGetChar();
    //TERM_PutChar(c);
    gps_receiveData(c);
}

CY_ISR(ISR_rpm_counter_tc)
{  
    uint32 isr_type = Timer_RPM_GetInterruptSource();
    Timer_RPM_ClearInterrupt(isr_type);
    Timer_RPM_TriggerCommand(Timer_RPM_MASK, Timer_RPM_CMD_RELOAD);
}

CY_ISR(ISR_millis)
{
    PWM_Bright_ClearInterrupt(PWM_Bright_INTR_MASK_TC);
    millis_count++;
     
    if (millis_count % 50) // Each 50ms
    {
        // Check for new GPS data
        if (gps_newDataAvailable())
        {
            gps_getData(&nav_data);
            rtcUpdate(nav_data.timestamp);
            /*
            switch(gps_getQuality())
            {
                case 0: status = 2; break;
                case 1: status = 3; break;
                case 2: status = 1; break;
                default: {}
            }*/
        }
        // Refresh display
        display_update(nav_data.speed/100, nav_data.course/100, update_rpm(), 3);
    }
}

/* [] END OF FILE */