/*

ISL 29125 with RGB cells 
can be easily adjusted to use other device and possibly more 
light options. 
Here yellow LED is superposition on R and G 
Blue cell is only to check for background light

Used before with Nordic BLE SDK 15.2 
running on nRF52840

App was detecting when sesor was covered with device under test
and three LED's were tested: Green,Yellow, Red.

The snipped indicates the code to be added in main.c

Also please add to paths 
components\libraries\twi_mngr //should you need to use it?
components\libraries\twi_sensor

also in sdk_config.h in section nRF drivers
#define TWI_ENABLED 1
#define TWI1_ENABLED 1
#define TWI1_USE_EASY_DMA 1

*/


#include "nrf_drv_twi.h"
#include  "ISL29125_reg.h"

//====  TWI instance ID. 

#define TWI_INSTANCE_ID     1

//====  TWI instance. 

static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

static volatile bool m_xfer_done = false;
static volatile bool TWI_OK = false;

#define APP_TWI_TIMEOUT         5000 

//static uint8_t TWI_ADDR =    If more than one I2C device in use 
//then it is not const in following functions and neet to set on each 
//change of device

#define TWI_ADDR	ISL29125_DEVICE_ADDR 

//======================================
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{ if ( (p_event->type) == NRF_DRV_TWI_EVT_DONE )   m_xfer_done = true;
}//======================================

//here TWI pins are defined 

void twi_init (void)
{ ret_code_t err_code;
  const nrf_drv_twi_config_t twi_config = {
       .scl                = ARDUINO_SCL_PIN,
       .sda                = ARDUINO_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };
    err_code = nrf_drv_twi_init(&m_twi, &twi_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);
    nrf_drv_twi_enable(&m_twi);
}

//read register over TWI
static uint8_t app_twi_read_reg(uint8_t reg) //uint8_t * p_rx_buffer, uint16_t m_length)
{  uint8_t value; 
	 uint32_t timeout;
	 m_xfer_done = false;
   timeout = APP_TWI_TIMEOUT;
	 (void)nrf_drv_twi_tx(&m_twi, TWI_ADDR, &reg, 1, true); //no stop = true!!
	  while ((!m_xfer_done) && timeout )   {   if (timeout) timeout--;     __WFE();    }
		
		 m_xfer_done = false;
	  timeout = APP_TWI_TIMEOUT;
   (void)nrf_drv_twi_rx(&m_twi, TWI_ADDR, &value, 1);
   while ((!m_xfer_done) && timeout )   {   if (timeout) timeout--;     __WFE();    }
	 return value;
}
/// Reads num bytes starting from reg into _buff array
void	app_twi_read_burst(uint8_t  reg, int num, uint8_t  * buff)   // &m_sample, sizeof(m_sample));
{	 	 uint32_t timeout;
  timeout = APP_TWI_TIMEOUT;
	m_xfer_done = false;
	(void)nrf_drv_twi_tx(&m_twi, TWI_ADDR, &reg, 1, true); //no stop = true!!
		  while ((!m_xfer_done) && timeout )   {   if (timeout) timeout--;     __WFE();    }
		
		 m_xfer_done = false;
	 timeout = APP_TWI_TIMEOUT;
   (void)nrf_drv_twi_rx(&m_twi, TWI_ADDR, buff, num);
   while ((!m_xfer_done) && timeout )   {   if (timeout) timeout--;     __WFE();    }
}

void	app_twi_cmd_write(uint8_t  address, uint8_t  val)
{  ret_code_t  err_code;
   uint8_t reg[2] = {address, val};    
	 uint32_t timeout = APP_TWI_TIMEOUT;
	 m_xfer_done = false;
	 err_code = nrf_drv_twi_tx(&m_twi, TWI_ADDR, reg, 2, false);   //write val to reg and stop
   APP_ERROR_CHECK(err_code);
	 while ((!m_xfer_done) && timeout )   {   if (timeout) timeout--;     __WFE();    }
}
//=== here just a simple wrap of twi access for light sensor
uint8_t ISL29125_reg_read(uint8_t reg)
{   return app_twi_read_reg(reg);
}
void ISL29125_reg_write(uint8_t reg, uint8_t val)
{  app_twi_cmd_write( reg, val);  
}

void ISL29125_reg_set_bit_pattern(uint8_t reg, uint8_t bit_pattern, uint8_t mask)
{   uint8_t reg_val;
    reg_val  = ISL29125_reg_read(reg);
    reg_val &= ~mask;
    reg_val |= bit_pattern;
    ISL29125_reg_write(reg, reg_val);//, sizeof(reg_val));
}

void ISL29125_reg_set_bit(uint8_t reg, uint8_t bits)
{    ISL29125_reg_set_bit_pattern(reg, bits, 0x00);
}
void ISL29125_reg_reset_bit(uint8_t reg, uint8_t bits)
{    ISL29125_reg_set_bit_pattern(  reg, 0x00, bits);
}
bool ISL_29125_whoami(void)
{ uint8_t reg;
  reg  = ISL29125_reg_read( ISL29125_DEVICE_ID );
  if ( reg == ISL29125_WAI_ID ) return true; else return false;
}
// Reset all registers - returns true if successful
bool ISL_29125_reset()
{
  uint8_t data = 0x00;
	//uint8_t status = 0x00;
  // Reset registers
  ISL29125_reg_write(ISL29125_DEVICE_ID, ISL29125_CMD_RESET);
	nrf_delay_ms(5);
  // Check reset
  data  = ISL29125_reg_read(ISL29125_CONFIG_1);
  data |= ISL29125_reg_read(ISL29125_CONFIG_2);
  data |= ISL29125_reg_read(ISL29125_CONFIG_3);
  data |= ISL29125_reg_read(ISL29125_STATUS);
  if (data != 0x00) // && STAUS 
  {
    return false;
  }
  return true;
}

//..try other (unofficial?) modes if you dare (copy from app notes)

/* CUSTOM MODE Initialization (1 time only):
Set CUSTOM Mode Enable 
W:0x00:0x89
W:0x00:0xC9
Read Value 
R:0x1D -> MEM
Enable High Sensitivity (access to CM_4 )
W:0x19:0x40
Write Value 
W:0x1B:MEM
Exit CUSTOM Mode 
W:0x00:0x00

Run Time

Set CUSTOM Mode Enable 
W:0x00:0x89
W:0x00:0xC9
Set High Sensitivity 
W:0x1C:0x03*ON
Exit CUSTOM Mode 
W:0x00:0x00
*/


void ISL29125_RGB(void)  
{	ISL29125_reg_set_bit(ISL29125_CONFIG_1, ISL29125_CFG1_MODE_RGB |  ISL29125_CFG1_10KLUX | ISL29125_CFG1_12BIT | ISL29125_CFG1_ADC_SYNC_NORMAL);}
/*
#define ISL29125_CFG1_MODE_POWERDOWN 0x00
#define ISL29125_CFG1_MODE_G 0x01
#define ISL29125_CFG1_MODE_R 0x02
#define ISL29125_CFG1_MODE_B 0x03
#define ISL29125_CFG1_MODE_STANDBY 0x04
#define ISL29125_CFG1_MODE_RGB 0x05
#define ISL29125_CFG1_MODE_RG 0x06
#define ISL29125_CFG1_MODE_GB 0x07
*/	
void ISL29125_CONV_INT(void)  
{	ISL29125_reg_set_bit(ISL29125_CONFIG_3, ISL29125_CFG3_RGB_CONV_TO_INT_ENABLE );}

static uint16_t RGB[3];

void ISL29125_Read_RGB(void) //actual order in this device is GRB, LSB 1st
{	uint8_t buf[6];     //G[LH]   R   B
	uint8_t status; //dummy read of status to clear int
	app_twi_read_burst(ISL29125_GREEN_L, 6, buf);
	status  = ISL29125_reg_read(ISL29125_STATUS);
  RGB[0] = buf[3] *256 + buf[2]; //red
  RGB[1] = buf[1] *256 + buf[0]; //green
  RGB[2] = buf[5] *256 + buf[4]; //blue	
}	
	
void ISL29125_Start(void)
{ if (ISL_29125_reset()  && ISL_29125_whoami() ) 
	TWI_OK = true; else TWI_OK = false;
	if (TWI_OK)
	{ ISL29125_RGB(); //scan all 3
		ISL29125_CONV_INT(); //INT pin when ADC complete
	}
}

//===================================
//example definition of interrupt pin 
#define ISL29125_INT ARDUINO_AREF_PIN 

//number of samples averaged
#define light_samples_count 128  

//data type for the averages stored
typedef struct
{ int SUM_R;
	int SUM_G;
	int SUM_B;
	int CNT;
}light_t;



//type for the light limits
typedef struct
{ int R;
	int G;
	int B;
}light_ref;

//enum of detected cases, as many as required in the app.
//at zero index should add one case more: undefined ?
typedef enum 
{
 Red_LED,
 Yellow_LED,
 Green_LED,
 Dark,
 Light,
} light_case_t;


light_case_t light_case=0; 

light_t RYGDL[5]; //runtime sum of light limits, size equal cases enum above

//the limits of light intensity, adjusted from readings in debug mode.
//these are application dependent and vary with different LED or other DUT 

const light_ref Light_Limits[] = {  //RGB limits to compare
	 [Red_LED    ] = { 0x400, 0x280, 0x180},
	 [Yellow_LED ] = { 0x100, 0x100, 0x80},
	 [Green_LED  ] = { 0x180, 0x300, 0x280},
	 [Dark       ] = { 0x20, 0x20, 0x20},   
	/// [Light      ] = in our case, complimentary to Dark!
};

//decide light case and add to it's averaging SUM
//this works also detecting reliably the 'blinks' and is very useful 

void check_light_case(void)
{			if ( ( RGB[0] < Light_Limits[Dark].R )  //0x20 ) 
       	&& ( RGB[1] < Light_Limits[Dark].G )  //0x20 ) 		
	      && ( RGB[2] < Light_Limits[Dark].B ) ) //0x20   ) )  	
	        light_case = Dark;    else
			if ( ( RGB[0] > Light_Limits[Red_LED].R ) //0x400 ) 	
				&& ( RGB[1] < Light_Limits[Red_LED].G ) //0x280 ) 	
			  && ( RGB[2] < Light_Limits[Red_LED].B ) ) //0x180  ) )	  
			    light_case = Red_LED; else
					  	//if ( ( RGB[0] > 0x240 ) 	&& ( RGB[1] > 0x240 ) 	&& ( RGB[2] < 0x1F0 ) )	
			if ( ( RGB[0] > Light_Limits[Yellow_LED].R ) //0x100 )	  
				&& ( RGB[1] > Light_Limits[Yellow_LED].G ) //0x100 )   
			  && ( RGB[2] < Light_Limits[Yellow_LED].B ) ) //0x80   ) )		
			    light_case = Yellow_LED; else
							//&& ( RGB[0] > RGB[2] ) 	&& ( RGB[1] > RGB[2] ) )		light_case = Yellow_LED; else	
			if ( ( RGB[0] < Light_Limits[Green_LED].R ) //0x180 ) 	
				&& ( RGB[1] > Light_Limits[Green_LED].G ) //0x300 ) 	
			  && ( RGB[2] < Light_Limits[Green_LED].B ) )//0x280  ) )	  
			    light_case = Green_LED; 
			    else	
				 	light_case = Light; //complimentary to Dark!
					
						RYGDL[light_case].SUM_R += RGB[0];
						RYGDL[light_case].SUM_G += RGB[1];
						RYGDL[light_case].SUM_B += RGB[2];
						RYGDL[light_case].CNT++;
}
//=======================
int main(void)
{  
	uint32_t err_code;
    	static int light_samples=0;
		
	//light sensor init	
	twi_init();
    ISL29125_Start();		
		nrf_gpio_cfg_input(ISL29125_INT,NRF_GPIO_PIN_PULLUP);	
		
    for (;;)
    {   
			
	  if ( ( nrf_gpio_pin_read(ISL29125_INT) == 0) && TWI_OK )
		{ 
			ISL29125_Read_RGB();
							
	        uint16_t RGBL[3];	// used to store averaged light readings for debug purposes 
			check_light_case();
						//count detection to decide colour reliably depending on scenario 
				
		  if (++light_samples >= light_samples_count)
			{ light_samples=0;		
		
		//the limits for yellow LED are specific and tricky to adjust due to lack of dedicated cell
		//and so here we check that it detected more times than other states	
        //and we check it 1st 		
				if ( ( RYGDL[Yellow_LED].CNT > RYGDL[Red_LED].CNT  )  && ( RYGDL[Yellow_LED].CNT > RYGDL[Green_LED].CNT  ) )
							{  
								 light_case = Yellow_LED; 
					//the line below is used by other part of code to detect the EXPECTED blink, 
					//when the app is told DUT to test the LED,
					//so the 'BLINK' detection is recorded and 
					//information not replaced by other state
								 if (LED_STATE == Yellow_WY) light_case_pass = Yellow_LED;
								
							}	else
							
					if ( RYGDL[Red_LED].CNT > 0 )  
							{  
								 light_case = Red_LED; 
							
								if (LED_STATE == Red_WR) light_case_pass = Red_LED;
															
							}	else

					if ( RYGDL[Green_LED].CNT > 0 )  
							{  
								 light_case = Green_LED; 
								
								 if (LED_STATE == Green_OK) light_case_pass = Green_LED;

							}	else
							
					if ( RYGDL[Dark].CNT > RYGDL[Light].CNT )  
							{  
								 light_case = Dark; 
								//DUT detection
				
							}	else
							{  
								 light_case = Light; 
	                            //DUT removal
								 		 
							}		
	                            RGBL[0] = (uint16_t)(RYGDL[light_case].SUM_R / RYGDL[light_case].CNT);
								RGBL[1] = (uint16_t)(RYGDL[light_case].SUM_G / RYGDL[light_case].CNT);
								RGBL[2] = (uint16_t)(RYGDL[light_case].SUM_B / RYGDL[light_case].CNT);
					
					memset(&RYGDL, 0, sizeof(RYGDL)); //all back to 0		
	        } //light samples				
			}//INT pin
	   } //for
}//main
//EOF
	
		