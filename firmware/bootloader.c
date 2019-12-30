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

#include "tinyprintf.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_flash.h"
#include "hw.h"
#include "sdcard.h"
#include "ff.h"
#include "diskio.h"

const uint32_t Upgrade_AppBaseAddress = 0x08010000; /*!< Base address of the Application Image */

typedef void (*pFunction)(void);

void app_launch(void)
{
  uint32_t JumpAddress;
  pFunction Jump_To_Application;

  NVIC_SetVectorTable(Upgrade_AppBaseAddress, 0);
      
  JumpAddress = *(__IO uint32_t*) (Upgrade_AppBaseAddress + 4);
  Jump_To_Application = (pFunction) JumpAddress;
  /* init app Stack Pointer */
  __set_MSP(*(__IO uint32_t*) Upgrade_AppBaseAddress);
      
  /* Jump! */
  Jump_To_Application();
  
  while (1);
}

void display_progress()
{
#define SPIN_FRAMES 7  
  static uint8_t frame = 0;
  const char spinner[] = "-\x1|/-\x1|/";
  char msg[] = "UPDATE..";

  msg[7] = spinner[frame++]; 

  hw_lcd_cmd(0x02);		/* home */
  hw_lcd_put_str(msg);
  
  if (frame > SPIN_FRAMES)
    frame = 0;

}

bool flash_format()
{
#define N_FIRMWARE_SECTORS 8
  
  FLASH_Status erasestatus;
  uint16_t sector[N_FIRMWARE_SECTORS], i;

  /* bootloader is first 4 sectors - 16Kb each = 64k */
  
  sector[0] = FLASH_Sector_4;
  sector[1] = FLASH_Sector_5;
  sector[2] = FLASH_Sector_6;
  sector[3] = FLASH_Sector_7;
  sector[4] = FLASH_Sector_8;
  sector[5] = FLASH_Sector_9;
  sector[6] = FLASH_Sector_10;
  sector[7] = FLASH_Sector_11;

  tfp_printf("erasing flash\n");
  
  for (i=0;i<N_FIRMWARE_SECTORS;i++)
    {
      erasestatus = FLASH_EraseSector(sector[i], VoltageRange_3);
      display_progress();
      tfp_printf("erased sector %i\n", i);
      if (erasestatus != FLASH_COMPLETE)
	return false;
    }

  return true;
}

bool flash_burn()
{
  FATFS fs;  
  FIL fsrc;  
  uint32_t br; 
  FRESULT res;

#define BUF_SZ 512
  char buffer[BUF_SZ];
  uint32_t offset = 0;
  
  disk_initialize(0);
  
  res = f_mount(0, &fs);

  if (res != FR_OK)
    {
      hw_lcd_clear();
      hw_lcd_put_str("No SD?");
      return false;
    }

  res = f_open(&fsrc, "/update.bin", FA_OPEN_EXISTING | FA_READ);

  if (res != FR_OK)
    {
      tfp_printf("f_open failed\n");
      hw_lcd_clear();
      hw_lcd_put_str("No FW?");
      return false;
    }

  if (!flash_format())
    {
      hw_lcd_put_str("FAIL");
      return false;
    }
  
  do {
    uint32_t data, pc;
    FLASH_Status status;

    tfp_printf("reading firmware file\n");
    
    res = f_read(&fsrc, (uint8_t *)buffer, BUF_SZ, (unsigned int*)&br);

    tfp_printf("read %i bytes\n", br);
    
    if (res == FR_OK && br > 0)
      {
	for (pc = 0; pc < br; pc += 4)
	  {
	    data = *((uint32_t *) (buffer + pc));
	    //tfp_printf("flashing %i at addr %i:%i\n", data, offset + pc, pc);
	    status = FLASH_ProgramWord(Upgrade_AppBaseAddress + offset + pc, data);
	    
	    if (status != FLASH_COMPLETE)
	      {
		tfp_printf("flash failed?\n");
		hw_lcd_clear();
		hw_lcd_put_str("FAIL :(");
		return false;
	      }
	  }
	offset += br;
	display_progress();
	//tfp_printf("flashed %i bytes\n", br);
      }
    
  } while (res == FR_OK && br == BUF_SZ);

  return true;
}



int main(void)
{
  int i = 0;
  
  hw_init_bank_button();

  while (!(GPIOA->IDR & GPIO_Pin_8) && i < 5000) /* button held */
    {
      i++;
    }

  if (i == 5000)
    {
      uint8_t slash[8] = {
	0b00000,
	0b10000,
	0b01000,
	0b00100,
	0b00010,
	0b00001,
	0b00000,
      };

      SysTick_Config(SystemCoreClock / 1000); /* Note interupts get turns off in main loop, this just for init */
      NVIC_SetPriority(SysTick_IRQn, 0x0);

      hw_usart_dbg_init();
      tfp_printf("bootloader up...\n");
      
      hw_lcd_init();

      hw_lcd_make_char(1, slash);
      
      FLASH_Unlock();

      if (flash_burn())
	{
	  hw_lcd_clear();
	  hw_lcd_put_str("Done!");
	  hw_delay_ms(2000);

	  NVIC_SystemReset();	/* reboot */
	}

      /* SOmething went wrong :( Just hang here for physical reboot */
      while (1)
	i++;
    }
  
  app_launch();
  
}
