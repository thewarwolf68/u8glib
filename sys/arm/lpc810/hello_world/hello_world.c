/*

  hello_world.c
  
  Universal 8bit Graphics Library
  
  Copyright (c) 2014, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  


*/



#include "chip.h"
#include "u8g.h"



#define SYS_CORE_CLOCK 12000000UL
#define SYS_TICK_PERIOD_IN_MS 50



/*=======================================================================*/
/* delay */

void delay_system_ticks(uint32_t sys_ticks);		/* from lpc81x_system.c */

/*
  Delay by the provided number of micro seconds.
  Limitation: "us" * System-Freq in MHz must now overflow in 32 bit.
  Values between 0 and 1.000.000 (1 second) are ok.
*/
void delay_micro_seconds(uint32_t us)
{
  uint32_t sys_ticks;

  sys_ticks = SYS_CORE_CLOCK / 1000000UL;
  sys_ticks *= us;
  delay_system_ticks(sys_ticks);  
}


/*=======================================================================*/
/* system procedures and sys tick master task */

volatile uint32_t sys_tick_irq_cnt=0;


void __attribute__ ((interrupt)) SysTick_Handler(void)
{
  sys_tick_irq_cnt++;
  
}


/*=======================================================================*/
/* u8g delay procedures */

void u8g_Delay(uint16_t val)
{
    
  delay_micro_seconds(1000UL*(uint32_t)val);
}

void u8g_MicroDelay(void)
{
  delay_micro_seconds(1);
}

void u8g_10MicroDelay(void)
{
  delay_micro_seconds(10);
}

/*=======================================================================*/
/* lpc810 i2c */

/*
taken from AN11329
*/
typedef struct
{
  __IO uint32_t  CFG;			  /* 0x00 */
  __IO uint32_t  STAT;
  __IO uint32_t  INTENSET;
  __O  uint32_t  INTENCLR;
  __IO uint32_t  TIMEOUT;		/* 0x10 */
  __IO uint32_t  DIV;
  __IO uint32_t  INTSTAT;
       uint32_t  Reserved0[1];  
  __IO uint32_t  MSTCTL;			  /* 0x20 */
  __IO uint32_t  MSTTIME;
  __IO uint32_t  MSTDAT;
       uint32_t  Reserved1[5];
  __IO uint32_t  SLVCTL;			  /* 0x40 */
  __IO uint32_t  SLVDAT;
  __IO uint32_t  SLVADR0;
  __IO uint32_t  SLVADR1;
  __IO uint32_t  SLVADR2;			  /* 0x50 */
  __IO uint32_t  SLVADR3;
  __IO uint32_t  SLVQUAL0;
       uint32_t  Reserved2[9];
  __I  uint32_t  MONRXDAT;			/* 0x80 */		
} LPC_I2C_TypeDef;

#define LPC_I2C               ((LPC_I2C_TypeDef    *) LPC_I2C_BASE   )

#define  I2C_CFG_MSTEN (0x1)
#define  I2C_CFG_SLVEN (0x2)
#define  I2C_STAT_MSTPENDING (0x1)
#define  I2C_STAT_MSTSTATE (0xe)
#define  I2C_STAT_MSTST_IDLE (0x0)
#define  I2C_STAT_MSTST_RX (0x2)
#define  I2C_STAT_MSTST_TX (0x4)
#define  I2C_STAT_MSTST_NACK_ADDR (0x6)
#define  I2C_STAT_MSTST_NACK_TX (0x8)
#define  I2C_STAT_SLVPENDING (0x100)
#define  I2C_STAT_SLVSTATE (0x600)
#define  I2C_STAT_SLVST_ADDR (0x000)
#define  I2C_STAT_SLVST_RX (0x200)
#define  I2C_STAT_SLVST_TX (0x400)
#define  I2C_MSTCTL_MSTCONTINUE (0x1)
#define  I2C_MSTCTL_MSTSTART (0x2)
#define  I2C_MSTCTL_MSTSTOP (0x4)
#define  I2C_SLVCTL_SLVCONTINUE (0x1)
#define  I2C_SLVCTL_SLVNACK (0x2)

static uint8_t lpc81x_i2c_err_code;
static uint8_t lpc81x_i2c_opt;		/* U8G_I2C_OPT_NO_ACK */

/*
  position values
    1: start condition
    2: sla transfer
*/
static uint8_t lpc81x_i2c_err_pos;


void lpc81x_i2c_clear_error(void)
{
  lpc81x_i2c_err_code = U8G_I2C_ERR_NONE;
  lpc81x_i2c_err_pos = 0;
}

uint8_t  lpc81x_i2c_get_error(void)
{
  return lpc81x_i2c_err_code;
}

uint8_t lpc81x_i2c_get_err_pos(void)
{
  return lpc81x_i2c_err_pos;
}

static void lpc81x_i2c_set_error(uint8_t code, uint8_t pos)
{
  if ( lpc81x_i2c_err_code > 0 )
    return;
  lpc81x_i2c_err_code |= code;
  lpc81x_i2c_err_pos = pos;
}


void lpc81x_i2c_init(uint8_t options)
{
  lpc81x_i2c_clear_error();
}

uint8_t lpc81x_i2c_wait(uint8_t mask, uint8_t pos)
{
  return 1;
}

uint8_t lpc81x_i2c_start(uint8_t sla)
{
  return 1;
}
uint8_t lpc81x_i2c_send_byte(uint8_t data)
{
  return 1;
}

void lpc81x_i2c_stop(void)
{
}


/*=======================================================================*/
/* u8glib com callback */


#define I2C_SLA         (0x3c*2)
//#define I2C_CMD_MODE  0x080
#define I2C_CMD_MODE    0x000
#define I2C_DATA_MODE   0x040

uint8_t u8g_a0_state;
uint8_t u8g_set_a0;

static uint8_t u8g_com_ssd_start_sequence(u8g_t *u8g)
{
  /* are we requested to set the a0 state? */
  if ( u8g_set_a0 == 0 )
    return 1;

  /* setup bus, might be a repeated start */
  if ( lpc81x_i2c_start(I2C_SLA) == 0 )
    return 0;
  if ( u8g_a0_state == 0 )
  {
    if ( lpc81x_i2c_send_byte(I2C_CMD_MODE) == 0 )
      return 0;
  }
  else
  {
    if ( lpc81x_i2c_send_byte(I2C_DATA_MODE) == 0 )
      return 0;
  }

  u8g_set_a0 = 0;
  return 1;
}

uint8_t u8g_com_ssd_i2c_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr)
{
  switch(msg)
  {
    case U8G_COM_MSG_INIT:
      //u8g_com_arduino_digital_write(u8g, U8G_PI_SCL, HIGH);
      //u8g_com_arduino_digital_write(u8g, U8G_PI_SDA, HIGH);
      //u8g_a0_state = 0;       /* inital RS state: unknown mode */
    
      lpc81x_i2c_init(u8g->pin_list[U8G_PI_I2C_OPTION]);

      break;
    
    case U8G_COM_MSG_STOP:
      break;

    case U8G_COM_MSG_RESET:
      /* Currently disabled, but it could be enable. Previous restrictions have been removed */
      /* u8g_com_arduino_digital_write(u8g, U8G_PI_RESET, arg_val); */
      break;
      
    case U8G_COM_MSG_CHIP_SELECT:
      u8g_a0_state = 0;
      u8g_set_a0 = 1;		/* force a0 to set again, also forces start condition */
      if ( arg_val == 0 )
      {
        /* disable chip, send stop condition */
	lpc81x_i2c_stop();
     }
      else
      {
        /* enable, do nothing: any byte writing will trigger the i2c start */
      }
      break;

    case U8G_COM_MSG_WRITE_BYTE:
      //u8g_set_a0 = 1;
      if ( u8g_com_ssd_start_sequence(u8g) == 0 )
	return lpc81x_i2c_stop(), 0;
      if ( lpc81x_i2c_send_byte(arg_val) == 0 )
	return lpc81x_i2c_stop(), 0;
      // lpc81x_i2c_stop();
      break;
    
    case U8G_COM_MSG_WRITE_SEQ:
      //u8g_set_a0 = 1;
      if ( u8g_com_ssd_start_sequence(u8g) == 0 )
	return lpc81x_i2c_stop(), 0;
      {
        register uint8_t *ptr = arg_ptr;
        while( arg_val > 0 )
        {
	  if ( lpc81x_i2c_send_byte(*ptr++) == 0 )
	    return lpc81x_i2c_stop(), 0;
          arg_val--;
        }
      }
      // lpc81x_i2c_stop();
      break;

    case U8G_COM_MSG_WRITE_SEQ_P:
      //u8g_set_a0 = 1;
      if ( u8g_com_ssd_start_sequence(u8g) == 0 )
	return lpc81x_i2c_stop(), 0;
      {
        register uint8_t *ptr = arg_ptr;
        while( arg_val > 0 )
        {
	  if ( lpc81x_i2c_send_byte(u8g_pgm_read(ptr)) == 0 )
	    return 0;
          ptr++;
          arg_val--;
        }
      }
      // lpc81x_i2c_stop();
      break;
      
    case U8G_COM_MSG_ADDRESS:                     /* define cmd (arg_val = 0) or data mode (arg_val = 1) */
      u8g_a0_state = arg_val;
      u8g_set_a0 = 1;		/* force a0 to set again */
    
      break;
  }
  return 1;
}


/*========================================================================*/
/* main */

u8g_t u8g;

void draw(uint8_t pos)
{
  u8g_SetFont(&u8g, u8g_font_6x10r);
  u8g_DrawStr(&u8g,  0, 12+pos, "Hello World!");
}

void u8g_main()
{
  uint8_t pos = 0;
  /*
    Please uncomment one of the displays below
    Notes:
      - "2x", "4x": high speed version, which uses more RAM
      - "hw_spi": All hardware SPI devices can be used with software SPI also.
	Access type is defined by u8g_com_hw_spi_fn
  */

  // u8g_InitComFn(&u8g, &u8g_dev_uc1701_dogs102_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_uc1701_dogs102_2x_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_uc1701_mini12864_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_uc1701_mini12864_2x_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_st7565_dogm132_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_st7565_dogm128_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_st7565_dogm128_2x_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_st7565_lm6059_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_st7565_lm6059_2x_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_st7565_lm6063_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_st7565_lm6063_2x_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_st7565_nhd_c12864_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_st7565_nhd_c12864_2x_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_st7565_nhd_c12832_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_st7565_64128n_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_st7565_64128n_2x_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_uc1601_c128032_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_uc1601_c128032_2x_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_uc1610_dogxl160_bw_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_uc1610_dogxl160_gr_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_uc1610_dogxl160_2x_bw_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_uc1610_dogxl160_2x_gr_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_pcd8544_84x48_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_pcf8812_96x65_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_ssd1325_nhd27oled_bw_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_ssd1325_nhd27oled_gr_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_ssd1325_nhd27oled_2x_bw_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_ssd1325_nhd27oled_2x_gr_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_ssd1327_96x96_gr_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_ssd1327_96x96_2x_gr_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_ssd1322_nhd31oled_bw_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_ssd1322_nhd31oled_2x_bw_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_ssd1322_nhd31oled_gr_hw_spi, u8g_com_hw_spi_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_ssd1322_nhd31oled_2x_gr_hw_spi, u8g_com_hw_spi_fn);
  u8g_InitComFn(&u8g, &u8g_dev_ssd1306_128x64_hw_spi, u8g_com_ssd_i2c_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_ssd1306_128x64_2x_hw_spi, u8g_com_ssd_i2c_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_ssd1309_128x64_hw_spi, u8g_com_ssd_i2c_fn);
  // u8g_InitComFn(&u8g, &u8g_dev_ssd1306_128x32_hw_spi, u8g_com_ssd_i2c_fn);
  
  // u8g_InitComFn(&u8g, &u8g_dev_ssd1306_128x32_2x_hw_spi, u8g_com_hw_spi_fn);
  
  
  u8g_SetDefaultForegroundColor(&u8g);

  for(;;)
  {
    /* picture loop */
    u8g_FirstPage(&u8g);
    do
    {
      draw(pos);
    } while ( u8g_NextPage(&u8g) );
    
    /* refresh screen after some delay */
    u8g_Delay(100);
    
    /* update position */
    pos++;
    pos &= 15;
  }  
}



/*=======================================================================*/
/* main procedure, called by "Reset_Handler" */

int __attribute__ ((noinline)) main(void)
{

  /* set systick and start systick interrupt */
  SysTick_Config(SYS_CORE_CLOCK/1000UL*(unsigned long)SYS_TICK_PERIOD_IN_MS);
  
  /* turn on GPIO */
  Chip_GPIO_Init(LPC_GPIO_PORT);

  /* disable SWCLK and SWDIO, after reset, boot code may activate this */
  Chip_SWM_DisableFixedPin(2);
  Chip_SWM_DisableFixedPin(3);
  
  /* turn on IOCON */
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);
  
  /* turn on switch matrix */
  Chip_SWM_Init();
  
  /* activate analog comperator */
  Chip_ACMP_Init(LPC_CMP);

  /* let LED on pin 4 of the DIP8 blink */
  Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, 2);  
  
  
  u8g_main();
    
  /*
  for(;;)
  {
    Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, 0, 2); 	
    delay_micro_seconds(500000UL);
    Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, 0, 2);    
    delay_micro_seconds(500000UL);
  }
  */

  
  /* enter sleep mode: Reduce from 1.4mA to 0.8mA with 12MHz */  
  while (1)
  {
    SCB->SCR |= (1UL << SCB_SCR_SLEEPONEXIT_Pos);		/* enter sleep mode after interrupt */ 
    Chip_PMU_SleepState(LPC_PMU);						/* enter sleep mode now */
  }
}
