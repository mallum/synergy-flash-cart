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

#pragma once

#include "stm32f4xx_conf.h"
#include "tinyprintf.h"

uint32_t hw_get_ms();

void hw_delay_ms(uint32_t ms);

void hw_delay_no_systick(uint32_t usec);


void hw_lcd_init();

void hw_lcd_put_char(uint8_t c);

void hw_lcd_put_str(char *s);

void hw_lcd_cmd(uint8_t c);

void hw_lcd_clear();

void hw_lcd_make_char(uint8_t index, uint8_t charmap[]);


void hw_usart_dbg_init();


void hw_init_gpio_output_enable(void);

void hw_init_gpio_addr(void);

void hw_init_gpio_data(void);

void hw_init_debug_leds(void);

void hw_init_bank_button(void);


enum sysclk_freq {
    SYSCLK_42_MHZ=0,
    SYSCLK_84_MHZ,
    SYSCLK_168_MHZ,
    SYSCLK_200_MHZ,
    SYSCLK_240_MHZ,
};


void hw_rcc_set_frequency(enum sysclk_freq freq);
