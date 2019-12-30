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

#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include <string.h>

#include "hw.h"

#include "sdcard.h"
#include "ff.h"
#include "diskio.h"

#define MAX_FILENAME_LEN 128
#define MAX_CART_FILENAMES 128

typedef struct _SynergyCartFile
{
  char      filename[MAX_FILENAME_LEN];
  uint16_t  size;
} SynergyCartFile;

SynergyCartFile _cart_files[MAX_CART_FILENAMES];
uint16_t _n_cart_files = 0;

#define CCRAM_SIZE (64 * 1024)
uint8_t _ccram[CCRAM_SIZE] __attribute__((section(".ccmram")));

#define ADDR_IN GPIOB->IDR
#define DATA_OUT GPIOA->ODR

void sort_carts()
{
  int i,j;

  for (i=0;i<_n_cart_files;i++)
    for (j=i+1;j<_n_cart_files; j++)
      if (strcmp(_cart_files[i].filename, _cart_files[j].filename) > 0)
	{
	  SynergyCartFile tmp;
	  /* meh.. */
	  memcpy((void*)tmp.filename, (void*)_cart_files[i].filename, MAX_FILENAME_LEN);
	  tmp.size = _cart_files[i].size;
	  memcpy((void*)_cart_files[i].filename, (void*)_cart_files[j].filename, MAX_FILENAME_LEN);
	  _cart_files[i].size = _cart_files[j].size;
	  memcpy((void*)_cart_files[j].filename, (void*)tmp.filename , MAX_FILENAME_LEN);
	  _cart_files[i].size = tmp.size;
	}
}

bool sd_load_cart(int index)
{
  volatile FATFS fs;   

  FIL fsrc;      
  uint32_t br, i = 0;
  FRESULT res;
  FILINFO finfo;
  uint8_t buf[256];
  char txt[9];
  
  res = f_open(&fsrc, _cart_files[index].filename, FA_OPEN_EXISTING | FA_READ);

  if (res != FR_OK)
    {
      tfp_printf("f_open failed\n");
      hw_lcd_clear();
      hw_lcd_put_str("SD Fail");
      return false;
    }

  do
    {
      res = f_read(&fsrc, (uint8_t *)buf, 256, (unsigned int*)&br);
      memcpy(&_ccram[i], buf, br);
      i += br;
    }
  while (res == FR_OK && br == 256 && i < CCRAM_SIZE);

  tfp_printf("read %i\n", i);

  /* fixme pull the name from the actual cart file itself */
  i = strlen(_cart_files[index].filename)-4;
  if (i>9) i=9;
  tfp_snprintf(txt, 9, "%s", _cart_files[index].filename+1);
  txt[i-1] = '\0';

  hw_lcd_clear();
  hw_lcd_put_str(txt);

  _cart_files[index].size = br;
  
  f_close(&fsrc);

  return true;
}

bool sd_get_carts(void)
{
  FATFS fs;  
  FIL fsrc;  
  uint32_t br, i; 
  FRESULT res;
  DIR dir;
  FILINFO fno;

  _n_cart_files = 0;
  
  disk_initialize(0);
  
  res = f_mount(0, &fs);

  if (res != FR_OK)
    {
      hw_lcd_clear();
      hw_lcd_put_str("SD Err!");
      return false;
    }
  
  res = f_opendir(&dir, "/");  /* Open the root */

  if (res == FR_OK)
    {
      while (true)
	{
	  res = f_readdir(&dir, &fno); /* Read a directory item */
	  if (res != FR_OK || fno.fname[0] == 0)
	    break;  /* Break on error or end of dir */
	  if (!(fno.fattrib & AM_DIR)) 
	    {
	      /* It is a file. */
	      i = strlen(fno.fname);
	      if (_n_cart_files < MAX_CART_FILENAMES && i > 4 && strncasecmp(".crt", (const char *)&fno.fname[i-4], 4) == 0)
		{
		  hw_lcd_clear();
		  hw_lcd_put_str(fno.fname);
		  tfp_snprintf(_cart_files[_n_cart_files++].filename, MAX_FILENAME_LEN, "/%s", fno.fname);
		}
            }
        }

      //f_closedir(&dir); no closedir in fatfs version used.. FIXME: need to update

      hw_lcd_clear();
      hw_lcd_put_str("Ready.");
    }

  if (_n_cart_files == 0)
    {
      hw_lcd_clear();
      hw_lcd_put_str("No CRTs!");
      return false;
    }

  sort_carts();
  
  return true;
}

void abort()		/* lazy disk problem stratergy.. reboot and rescan for card */
{
  while (GPIOA->IDR & GPIO_Pin_8)
    ;			/* loop unit button pressed */
  NVIC_SystemReset();	/* reboot */
}

int main(void)
{
  int     i = 0;
  bool    switch_was_low = true;
  uint8_t cart_index = 0;

  SysTick_Config(SystemCoreClock / 1000); /* Note interupts get turns off in main loop, this just for init */
  NVIC_SetPriority(SysTick_IRQn, 0x0);

  hw_rcc_set_frequency(SYSCLK_200_MHZ); /* turbo */

  /* CCM etc */
  RCC->APB2ENR |= 0 |  RCC_APB2ENR_SYSCFGEN ;
  SYSCFG->CMPCR |= SYSCFG_CMPCR_CMP_PD; 
  while ((SYSCFG->CMPCR & SYSCFG_CMPCR_READY) == 0);

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CCMDATARAMEN, ENABLE); 

  /* debug */
  hw_usart_dbg_init();
  tfp_printf("App up...\n");	/* debug, to serial */

  /* lcd */
  hw_lcd_init();
  tfp_printf("lcd inited...\n");
  
  hw_lcd_put_str("S");	/* Anim here...? */
  hw_delay_ms(100);
  hw_lcd_put_str("S");
  hw_delay_ms(100);
  hw_lcd_put_str("R");
  hw_delay_ms(100);
  hw_lcd_put_str("C");
  hw_delay_ms(500);

  /* Synergy interface pins up */  
  hw_init_gpio_data();
  hw_init_gpio_addr();
  hw_init_gpio_output_enable();

  /* SD card */
  if (!sd_get_carts())
    abort();

  if (!sd_load_cart(0))		
    abort();
    
  DATA_OUT = 0;

  __disable_irq();
  
  while (1)
    {
      if (!(GPIOC->IDR & GPIO_Pin_1))
	{
	  //GPIOA->BSRRL = GPIO_Pin_10; /* instrument - approx 120ns for a read.. OE cycle is approx 750ns  */
	  DATA_OUT = ((uint16_t)_ccram[ADDR_IN]) /* should really be DATA_OUT = DATA_OUT | _ccram[addr] .. but slower */;
	  //GPIOA->BSRRH = GPIO_Pin_10;
	}

      if (!(GPIOA->IDR & GPIO_Pin_8))
	{
	  if (switch_was_low)
	    {
	      if (++cart_index >= _n_cart_files)
		cart_index = 0;

	      if (!sd_load_cart(cart_index))
		abort();

	      switch_was_low = false;
	    }
	}
      else
	switch_was_low = true;
    }
  
}
