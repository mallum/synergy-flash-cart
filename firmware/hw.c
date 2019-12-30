/* 

Copyright (c) 2019 Matthew Allum / Allum & Co ltd. 

All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
  
#include "hw.h"

void NMI_Handler(void)
{
  while (1)
  {
  };
}

void HardFault_Handler(void)
{
  while (1)
  {
  };
}

void MemManage_Handler(void)
{
  while (1)
  {
  };
}

void BusFault_Handler(void)
{
  while (1)
  {
  };
}

void UsageFault_Handler(void)
{
  while (1)
  {
  };
}

void SVC_Handler(void)
{
  while (1)
  {
  };
}

void DebugMon_Handler(void)
{
  while (1)
  {
  };
}

void PendSV_Handler(void)
{
  while (1)
  {
  };
}


/* systick timing */

volatile uint32_t __msec;

uint32_t hw_get_ms()
{
  return __msec;
}

void hw_delay_ms(uint32_t ms)
{
  uint32_t MSS = __msec;
  while((__msec-MSS)<ms) asm volatile ("nop");
}

void hw_delay_no_systick(uint32_t usec) /* FIXME: very approx! */
{
  __IO uint32_t count = 0;
  const uint32_t utime = (120 * usec / 7);
  do
    {
      if ( ++count > utime )
	{
	  return ;
	}
    }
  while (1);
}

void SysTick_Handler(void)
{
  __msec++;
}

/* display */

#define LCD_PORT GPIOC

#define LCD_RS_PIN GPIO_Pin_0

#define LCD_DB4_PIN GPIO_Pin_2
#define LCD_DB5_PIN GPIO_Pin_3
#define LCD_DB6_PIN GPIO_Pin_4
#define LCD_DB7_PIN GPIO_Pin_5

#define LCD_ENABLE_PIN GPIO_Pin_6

enum LCD_FLAGS {
  LCD_COMMAND = 0x00,
  LCD_DATA = 0x10,
  LCD_CLEAR = 0x01,
  LCD_HOME = 0x02,
  LCD_ENTRY_MODE = 0x04,
  LCD_DISPLAY_STATUS = 0x08,
  LCD_CURSOR = 0x10,
  LCD_FUNCTION_SET = 0x20,
  LCD_SET_CGRAM_ADDRESS = 0x40,
  LCD_SET_DDRAM_ADDRESS = 0x80,

  LCD_SHIFT = 0x01,
  LCD_NO_SHIFT = 0x00,
  LCD_CURSOR_INCREMENT = 0x02,
  LCD_CURSOR_NO_INCREMENT = 0x00,
  LCD_DISPLAY_ON = 0x04,
  LCD_DISPLAY_OFF = 0x00,
  LCD_CURSOR_ON = 0x02,
  LCD_CURSOR_OFF = 0x00,
  LCD_BLINKING_ON = 0x01,
  LCD_BLINKING_OFF = 0x00,

  LCD_8_BITS = 0x10,
  LCD_4_BITS = 0x00,

  LCD_2_LINE = 0x08,
  LCD_1_LINE = 0x00,

  LCD_LARGE_FONT = 0x04,
  LCD_SMALL_FONT = 0x00,
};

void hw_lcd_strobe(void)
{
  hw_delay_no_systick(1000);
  GPIO_SetBits(LCD_PORT, LCD_ENABLE_PIN);
  hw_delay_no_systick(1000);
  GPIO_ResetBits(LCD_PORT, LCD_ENABLE_PIN);
}

void hw_lcd_send_nibble(uint8_t c)
{
  if(c & 0x8)
    GPIO_SetBits(LCD_PORT, LCD_DB7_PIN);
  else
    GPIO_ResetBits(LCD_PORT, LCD_DB7_PIN);

  if(c & 0x4)
    GPIO_SetBits(LCD_PORT, LCD_DB6_PIN);
  else
    GPIO_ResetBits(LCD_PORT, LCD_DB6_PIN);

  if(c & 0x2)
    GPIO_SetBits(LCD_PORT, LCD_DB5_PIN);
  else
    GPIO_ResetBits(LCD_PORT, LCD_DB5_PIN);

  if(c & 0x1)
    GPIO_SetBits(LCD_PORT, LCD_DB4_PIN);
  else
    GPIO_ResetBits(LCD_PORT, LCD_DB4_PIN);
}

void hw_lcd_data(uint8_t c)
{
  GPIO_SetBits(LCD_PORT, LCD_RS_PIN);
  hw_lcd_send_nibble(c>>4);
  hw_lcd_strobe();
  hw_lcd_send_nibble(c);
  hw_lcd_strobe();
  GPIO_ResetBits(LCD_PORT, LCD_RS_PIN);
}

void hw_lcd_cmd(uint8_t c)
{
  GPIO_ResetBits(LCD_PORT, LCD_RS_PIN);
  hw_lcd_send_nibble(c>>4);
  hw_lcd_strobe();
  hw_lcd_send_nibble(c);
  hw_lcd_strobe();
}

void
hw_lcd_init()
{
  GPIO_InitTypeDef g;
  uint8_t i;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

  g.GPIO_Pin = LCD_DB4_PIN|LCD_RS_PIN|LCD_DB5_PIN|LCD_DB6_PIN|LCD_DB7_PIN|LCD_ENABLE_PIN;
  g.GPIO_Mode = GPIO_Mode_OUT;
  g.GPIO_OType = GPIO_OType_PP;
  g.GPIO_Speed = GPIO_Speed_50MHz;
  g.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(LCD_PORT, &g);

  GPIO_ResetBits(LCD_PORT, LCD_DB4_PIN|LCD_RS_PIN|LCD_DB5_PIN|LCD_DB6_PIN|LCD_DB7_PIN|LCD_ENABLE_PIN);

  hw_delay_ms(50); // wait for warm up

  // Set to 4 bit operation.
  for (i = 0; i < 3; ++i)
    {
      hw_lcd_cmd ((LCD_FUNCTION_SET | LCD_8_BITS) >> 4);
      hw_delay_ms(5);
    }
  hw_lcd_cmd ((LCD_FUNCTION_SET | LCD_4_BITS) >> 4);
  
  hw_delay_ms(20);
  
  /* 1 line */
  hw_lcd_cmd (LCD_FUNCTION_SET | LCD_4_BITS | LCD_1_LINE | LCD_SMALL_FONT);

  hw_lcd_cmd (LCD_DISPLAY_STATUS | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINKING_OFF);
  hw_lcd_cmd (LCD_ENTRY_MODE | LCD_CURSOR_INCREMENT | LCD_NO_SHIFT);
  hw_lcd_cmd (LCD_CLEAR);
  hw_lcd_cmd (LCD_HOME);
}

void hw_lcd_put_char(uint8_t c)
{
  if  ( (c < 8)
	|| ((c >= 0x20) && (c <= 0x7f))
	|| ((c >= 0xa0) && (c <= 0xff))) // check if 'c' is within display boundry
    {
      hw_lcd_data(c);
    }
}

void hw_lcd_put_str(char *s)
{
  uint8_t i=0;
  
  while(s[i] != '\0' && i < 8)
    hw_lcd_put_char(s[i++]);
}

void hw_lcd_clear()
{
  hw_lcd_cmd(0x01);
}

void hw_lcd_make_char(uint8_t index, uint8_t charmap[])
{
  index &= 0x7; // we only have 8 locations 0-7
  hw_lcd_cmd(LCD_SET_CGRAM_ADDRESS | (index << 3));

  for (int i=0; i<8; i++)
    hw_lcd_data(charmap[i]);
}

void _hw_usart_init(int baudrate)
{
  GPIO_InitTypeDef g;
  USART_InitTypeDef usart_init;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
  // sort out clocks
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

  /* Configure USART2 Tx (PA.02) as alternate function push-pull */
  g.GPIO_Pin = GPIO_Pin_9;
  g.GPIO_Speed = GPIO_Speed_50MHz;
  g.GPIO_Mode = GPIO_Mode_AF;
  g.GPIO_OType = GPIO_OType_PP;
  g.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &g);

  // Map USART2 to A.02
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
  // Initialize USART

  usart_init.USART_BaudRate = baudrate;
  usart_init.USART_WordLength = USART_WordLength_8b;
  usart_init.USART_StopBits = USART_StopBits_1;
  usart_init.USART_Parity = USART_Parity_No;
  usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  usart_init.USART_Mode = USART_Mode_Tx;

  /* Configure USART */
  USART_Init(USART1, &usart_init);
  /* Enable the USART */
  USART_Cmd(USART1, ENABLE);
}


static void USART_putc(USART_TypeDef* USARTx, unsigned char c)
{
  while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);

  USART_SendData(USARTx, c);
}

void hw_usart_dbg_init()
{
  _hw_usart_init(115200);
  init_printf((void*)USART1,(putcf)USART_putc);
}

/* usart end */


void hw_init_gpio_output_enable(void)
{
  GPIO_InitTypeDef  g;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

  g.GPIO_Pin = GPIO_Pin_1;
  g.GPIO_Mode = GPIO_Mode_IN;
  g.GPIO_OType = GPIO_OType_PP;
  g.GPIO_Speed = GPIO_Speed_100MHz;
  g.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOC, &g);
}

void hw_init_gpio_addr(void)
{
  GPIO_InitTypeDef  g;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  g.GPIO_Pin = 
    GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | 
    GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | 
    GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | 
    GPIO_Pin_12;
  g.GPIO_Mode = GPIO_Mode_IN;
  g.GPIO_OType = GPIO_OType_PP;
  g.GPIO_Speed = GPIO_Speed_100MHz;
  g.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOB, &g);
}

void hw_init_gpio_data(void)
{
  GPIO_InitTypeDef  g;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
  g.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | 
    GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_10 ; /* 10 for debug */
  g.GPIO_Mode = GPIO_Mode_OUT;
  g.GPIO_OType = GPIO_OType_PP;
  g.GPIO_Speed = GPIO_Speed_100MHz;
  g.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &g);
}

void hw_init_debug_leds()
{
  GPIO_InitTypeDef g;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  g.GPIO_Pin = GPIO_Pin_15;
  g.GPIO_Mode = GPIO_Mode_OUT;
  g.GPIO_Speed = GPIO_Speed_50MHz;
  g.GPIO_OType = GPIO_OType_PP;
  g.GPIO_PuPd = GPIO_PuPd_UP;
   
  GPIO_Init(GPIOB, &g);
}

void hw_init_bank_button()
{
  GPIO_InitTypeDef  g;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
  g.GPIO_Pin = GPIO_Pin_8;
  g.GPIO_Mode = GPIO_Mode_IN;
  g.GPIO_OType = GPIO_OType_PP;
  g.GPIO_Speed = GPIO_Speed_100MHz;
  g.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOA, &g);
}

void hw_rcc_set_frequency(enum sysclk_freq freq)
{
  int freqs[]   = {42, 84, 168, 200, 240};
 
  /* USB freqs: 42MHz, 42Mhz, 48MHz, 50MHz, 48MHz */
  int pll_div[] = {2, 4, 7, 10, 10}; 
 
  /* PLL_VCO = (HSE_VALUE / PLL_M) * PLL_N */
  /* SYSCLK = PLL_VCO / PLL_P */
  /* USB OTG FS, SDIO and RNG Clock =  PLL_VCO / PLLQ */
  uint32_t PLL_P = 2;
  uint32_t PLL_N = freqs[freq] * 2;
  uint32_t PLL_M = (HSE_VALUE/1000000);
  uint32_t PLL_Q = pll_div[freq];
 
  RCC_DeInit();
 
  /* Enable HSE osscilator */
  RCC_HSEConfig(RCC_HSE_ON);
 
  if (RCC_WaitForHSEStartUp() == ERROR) {
    return;
  }
  
  /* Configure PLL clock M, N, P, and Q dividers */
  RCC_PLLConfig(RCC_PLLSource_HSE, PLL_M, PLL_N, PLL_P, PLL_Q);
  
  /* Enable PLL clock */
  RCC_PLLCmd(ENABLE);
  
  /* Wait until PLL clock is stable */
  while ((RCC->CR & RCC_CR_PLLRDY) == 0);
  
  /* Set PLL_CLK as system clock source SYSCLK */
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  
  /* Set AHB clock divider */
  RCC_HCLKConfig(RCC_SYSCLK_Div1);
  
  //FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_5WS;
  
  /* Set APBx clock dividers */
  switch (freq) {
    /* Max freq APB1: 42MHz APB2: 84MHz */
  case SYSCLK_42_MHZ:
    RCC_PCLK1Config(RCC_HCLK_Div1); /* 42MHz */
    RCC_PCLK2Config(RCC_HCLK_Div1); /* 42MHz */
    break;
  case SYSCLK_84_MHZ:
    RCC_PCLK1Config(RCC_HCLK_Div2); /* 42MHz */
    RCC_PCLK2Config(RCC_HCLK_Div1); /* 84MHz */
    break;
  case SYSCLK_168_MHZ:
    RCC_PCLK1Config(RCC_HCLK_Div4); /* 42MHz */
    RCC_PCLK2Config(RCC_HCLK_Div2); /* 84MHz */
    break;
  case SYSCLK_200_MHZ:
    RCC_PCLK1Config(RCC_HCLK_Div4); /* 50MHz */
    RCC_PCLK2Config(RCC_HCLK_Div2); /* 100MHz */
    break;
  case SYSCLK_240_MHZ:
    RCC_PCLK1Config(RCC_HCLK_Div4); /* 60MHz */
    RCC_PCLK2Config(RCC_HCLK_Div2); /* 120MHz */
    break;
  }
  
  /* Update SystemCoreClock variable */
  SystemCoreClockUpdate();
}

