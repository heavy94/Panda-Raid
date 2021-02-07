// #1 Heading
// #2 RPM
// #3 Speed

#include "display.h"

//Private functions declaration
void to_7seg(uint32 val, uint8 * seg);

//Public functions definition
void display_init()
{
    DISP_SPI_Start();
    DISP_SPI_EnableTxInt();
}

void display_test()
{
    display_update(999,999,5999,2);
}

void display_update(uint32 speed, uint32 heading, uint32 rpm, uint8 status)
{
    uint8 tx_array[9]; //BGR
    uint8 seg_cen = 0;
    uint8 seg_ten = 0;
    uint8 seg_uni = 0;
    uint32 seg_rpm = 0; //Rightmost value represents de first LED
 
    //Divide speed in individual digits
    uint32 speed_cen = speed / 100;
    uint32 speed_ten = (speed % 100) / 10;
    uint32 speed_uni = speed % 10;
    //Divide heading in individual digits
    uint32 heading_cen = heading / 100;
    uint32 heading_ten = (heading % 100) / 10;
    uint32 heading_uni = heading % 10;
    //Map rpm to number of LEDs
    uint32 rpm_led = rpm / 250;
    
    //If the value of status is unbounded default to 0. Possible values are:
    // 0 -> off, 1 -> green, 2 -> red, 3 -> yellow
    if (status > 3)
    {
        status = 0;
    }
    
    //#1 Heading display
    //Obtain the 7seg representation of each digit
    to_7seg(heading_cen, &seg_cen);
    to_7seg(heading_ten, &seg_ten);
    to_7seg(heading_uni, &seg_uni);
    //Map each segment to its corresponding position
    //B
    tx_array[0] = (F_MASK(seg_uni) << 7) | (A_MASK(seg_ten) << 6) | (B_MASK(seg_cen) << 5) | (G_MASK(seg_cen) << 4) |
                  (E_MASK(seg_cen) << 3) | (C_MASK(seg_ten) << 1) | D_MASK(seg_uni);
    //G
    tx_array[1] = (A_MASK(seg_uni) << 7) | (B_MASK(seg_ten) << 6) | (G_MASK(seg_ten) << 5) | (F_MASK(seg_cen) << 4) |
                  (D_MASK(seg_cen) << 3) | (E_MASK(seg_ten) << 2) | C_MASK(seg_uni);
    //R - Bit 0 drives status led Green channel
    tx_array[2] = (B_MASK(seg_uni) << 7) | (G_MASK(seg_uni) << 6) | (F_MASK(seg_ten) << 5) | (A_MASK(seg_cen) << 4) |
                  (C_MASK(seg_cen) << 3) | (D_MASK(seg_ten) << 2) | (E_MASK(seg_uni) << 1) | STATUS_GREEN_POS(status);

    //#2 RPM bar
    seg_rpm = 0;
    for (uint8 i = 0; i < rpm_led; i++)
    {
        seg_rpm = (seg_rpm << 1) | 1;
    }
    //B
    tx_array[3] = (BIT_POS_21(seg_rpm) << 7) | (BIT_POS_18(seg_rpm) << 6) | (BIT_POS_15(seg_rpm) << 5) | (BIT_POS_12(seg_rpm) << 4) |
                  (BIT_POS_11(seg_rpm) << 3) | (BIT_POS_8(seg_rpm) << 2) | (BIT_POS_5(seg_rpm) << 1) | BIT_POS_2(seg_rpm);
    //G
    tx_array[4] = (BIT_POS_22(seg_rpm) << 7) | (BIT_POS_19(seg_rpm) << 6) | (BIT_POS_16(seg_rpm) << 5) | (BIT_POS_13(seg_rpm) << 4) |
                  (BIT_POS_10(seg_rpm) << 3) | (BIT_POS_7(seg_rpm) << 2) | (BIT_POS_4(seg_rpm) << 1) | BIT_POS_1(seg_rpm);
    //R
    tx_array[5] = (BIT_POS_23(seg_rpm) << 7) | (BIT_POS_20(seg_rpm) << 6) | (BIT_POS_17(seg_rpm) << 5) | (BIT_POS_14(seg_rpm) << 4) |
                  (BIT_POS_9(seg_rpm) << 3) | (BIT_POS_6(seg_rpm) << 2) | (BIT_POS_3(seg_rpm) << 1) | BIT_POS_0(seg_rpm);

    //#3 Speed display
    //Obtain the 7seg representation of each digit
    to_7seg(speed_cen, &seg_cen);
    to_7seg(speed_ten, &seg_ten);
    to_7seg(speed_uni, &seg_uni);
    //Map each segment to its corresponding position
    //B - Bit 3 drives status led Red channel
    tx_array[6] = (E_MASK(seg_cen) << 7) | (C_MASK(seg_cen) << 6) | (A_MASK(seg_ten) << 5) | (D_MASK(seg_ten) << 4) |
                  (STATUS_RED_POS(status) << 3) | (B_MASK(seg_uni) << 2) | (E_MASK(seg_uni) << 1) | C_MASK(seg_ten);
    //G
    tx_array[7] = (F_MASK(seg_cen) << 7) | (B_MASK(seg_cen) << 5) | (E_MASK(seg_ten) << 4) | (C_MASK(seg_uni) << 3) |
                  (A_MASK(seg_uni) << 2) | (D_MASK(seg_uni) << 1) | G_MASK(seg_ten);
    //R
    tx_array[8] = (A_MASK(seg_cen) << 7) | (D_MASK(seg_cen) << 6) | (G_MASK(seg_cen) << 5) | (F_MASK(seg_ten) << 4) |
                  (G_MASK(seg_uni) << 3) | (F_MASK(seg_uni) << 2) | B_MASK(seg_ten);
    
    
    //DATA TRANSMISSION
    //CyPins_SetPin(OE_0);
    //CyPins_SetPin(OE_1);
    //CyPins_SetPin(OE_2);
    
    DISP_SPI_WriteTxData(tx_array[8]);
    DISP_SPI_WriteTxData(tx_array[7]);
    DISP_SPI_WriteTxData(tx_array[6]);
    
    DISP_SPI_WriteTxData(tx_array[5]);
    DISP_SPI_WriteTxData(tx_array[4]);
    DISP_SPI_WriteTxData(tx_array[3]);
    
    DISP_SPI_WriteTxData(tx_array[2]);
    DISP_SPI_WriteTxData(tx_array[1]);
    DISP_SPI_WriteTxData(tx_array[0]);
    
    CyDelayUs(50);
    CyPins_SetPin(DISP_LOAD_0);
    CyDelayUs(50);
    CyPins_ClearPin(DISP_LOAD_0);
    
    //CyPins_ClearPin(OE_0);
    //CyPins_ClearPin(OE_1);
    //CyPins_ClearPin(OE_2);
    //DATA TRANSMISSION
}

//Private functions definition
void to_7seg(uint32 val, uint8 * seg) //abcdefg DP
{
    *seg = 0; //Check endianess

    switch(val)
    {
        case 0:
            *seg = 0b11111100;
            break;
        case 1:
            *seg = 0b01100000;
            break;
        case 2:
            *seg = 0b11011010;
            break;
        case 3:
            *seg = 0b11110010;
            break;
        case 4:
            *seg = 0b01100110;
            break;
        case 5:
            *seg = 0b10110110;
            break;
        case 6:
            *seg = 0b10111110;
            break;
        case 7:
            *seg = 0b11100000;
            break;
        case 8:
            *seg = 0b11111110;
            break;
        case 9:
            *seg = 0b11110110;
            break;
        default:
            *seg = 0;
    }
}