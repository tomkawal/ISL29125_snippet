/*
The MIT License (MIT)
Copyright (c) 2023 tomkawal

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

 #ifndef ISL29125_REG_H
 #define ISL29125_REG_H
 
 #include <stdint.h>

 #ifdef __cplusplus
 extern "C" {
 #endif
	 
//I2C ADDRESS
#define ISL29125_DEVICE_ADDR 0x44 	 
	 
	 
// ISL29125 Registers
#define ISL29125_DEVICE_ID 		0x00   //also Reset and CUSTOM MODE switch
#define ISL29125_CONFIG_1 		0x01
#define ISL29125_CONFIG_2 		0x02
#define ISL29125_CONFIG_3 		0x03
#define ISL29125_THR_LL 			0x04   //16 bit thresholds for interrupts
#define ISL29125_THR_LH 			0x05
#define ISL29125_THR_HL 			0x06
#define ISL29125_THR_HH 			0x07
#define ISL29125_STATUS 			0x08   //read status on any int to clear it
#define ISL29125_GREEN_L 		0x09   //16 bit colour registers
#define ISL29125_GREEN_H 		0x0A
#define ISL29125_RED_L 			0x0B
#define ISL29125_RED_H 			0x0C
#define ISL29125_BLUE_L 			0x0D
#define ISL29125_BLUE_H 			0x0E
	 
//ADVANCED SETTINGS REGISTERS (for greater sensitivity or higher clock speed)
#define ISL29125_CM_1			0x19
#define ISL29125_CM_2			0x1a
#define ISL29125_CM_3			0x1b
#define ISL29125_CM_4			0x1c
#define ISL29125_CM_5			0x1d	 
	 
//registers bits and values /commands
//WAI value :  ID in REG_ID
#define   ISL29125_WAI_ID 			0x7d     // READ value from register 0
#define 	ISL29125_CMD_RESET   	0x46    // causes reset written to register 0
    //write successively to register 0 for custom mode
#define 	ISL29125_CM_CMD1 		0x89    
#define   ISL29125_CM_CMD2 		0xC9

// Configuration Settings
#define   ISL29125_CFG_DEFAULT 0x00

// CONFIG1  DEF 0x00
// MODE 2:0
// Pick a mode, determines what color[s] the sensor samples, if any
#define ISL29125_CFG1_MODE_POWERDOWN 0x00
#define ISL29125_CFG1_MODE_G 0x01
#define ISL29125_CFG1_MODE_R 0x02
#define ISL29125_CFG1_MODE_B 0x03
#define ISL29125_CFG1_MODE_STANDBY 0x04
#define ISL29125_CFG1_MODE_RGB 0x05
#define ISL29125_CFG1_MODE_RG 0x06
#define ISL29125_CFG1_MODE_GB 0x07

// Light intensity range   
// In a dark environment 375Lux is best, otherwise 10KLux is likely the best option
#define ISL29125_CFG1_375LUX 0x00
#define ISL29125_CFG1_10KLUX 0x08

// Change this to 12 bit if you want less accuracy, but faster sensor reads
// At default 16 bit, sample update for a given color is about ~100ms
// at 12 bit it is 6.25 ms
// in FAST mode, 4 times speedup
#define ISL29125_CFG1_16BIT 0x00
#define ISL29125_CFG1_12BIT 0x10

// Unless you want the interrupt pin to be an input that triggers sensor sampling, leave this on normal
#define ISL29125_CFG1_ADC_SYNC_NORMAL 0x00     // ADC start at I2C write to CONFIG1
#define ISL29125_CFG1_ADC_SYNC_TO_INT 0x20     // ADC start at rising INT

// CONFIG2: DEF 0x00
//ACTIVE INFRARED (IR) COMPENSATION, with it high range can reach more than 10,000 lux.
// Selects upper or lower range of IR filtering
#define ISL29125_CFG2_IR_OFFSET_OFF 0x00    //0 to 63 codes
#define ISL29125_CFG2_IR_OFFSET_ON 0x80  	//106 to 169

// Sets amount of IR filtering, can use these presets or any value between 0x00 and 0x3F
// Consult datasheet for detailed IR filtering calibration
#define ISL29125_CFG2_IR_ADJUST_LOW 0x00
#define ISL29125_CFG2_IR_ADJUST_MID 0x20
#define ISL29125_CFG2_IR_ADJUST_HIGH 0x3F

// CONFIG3 - DEF 0x00
// No interrupts, or interrupts based on a selected color
#define ISL29125_CFG3_NO_INT 0x00   //DEF
#define ISL29125_CFG3_G_INT 0x01
#define ISL29125_CFG3_R_INT 0x02
#define ISL29125_CFG3_B_INT 0x03

// How many times a sensor sample must hit a threshold before triggering an interrupt
// More consecutive samples means more times between interrupts, but less triggers from short transients
#define ISL29125_CFG3_INT_PRST1 0x00		//DEF
#define ISL29125_CFG3_INT_PRST2 0x04
#define ISL29125_CFG3_INT_PRST4 0x08
#define ISL29125_CFG3_INT_PRST8 0x0C

// If you would rather have interrupts trigger when a sensor sampling is complete, enable this
// If this is disabled, interrupts are based on comparing sensor data to threshold settings
#define ISL29125_CFG3_RGB_CONV_TO_INT_DISABLE 0x00
#define ISL29125_CFG3_RGB_CONV_TO_INT_ENABLE  0x10

// STATUS  -  DEF: 0x04 after Power ON
// FLAG MASKS
#define ISL29125_FLAG_INT 0x01         //this bit goes high with INT pin Low until STATUS read on I2C
#define ISL29125_FLAG_CONV_DONE 0x02  //conversion end
#define ISL29125_FLAG_BROWNOUT 0x04   //Power up

#define ISL29125_FLAG_CONV_G 0x10     //under conversion
#define ISL29125_FLAG_CONV_R 0x20
#define ISL29125_FLAG_CONV_B 0x30
 
 #ifdef __cplusplus
 }
 #endif
 
#endif /* ISL29125_REG_H */	 
