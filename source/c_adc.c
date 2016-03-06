/*
Copyright (c) 2016 Robert Wolterman

Original BBIO Author Justin Cooper
Modified for CHIP_IO Author Robert Wolterman

Copyright (c) 2013 Adafruit
Author: Justin Cooper

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "c_adc.h"
#include "common.h"

#define LRADC        0x01C22800
#define CTRL_OFFSET  0x00
#define INTC_OFFSET  0x04
#define INTS_OFFSET  0x08
#define DATA0_OFFSET 0x0C
#define DATA1_OFFSET 0x10
#define LRADC_CTRL_DEFAULT 0x01000168
#define LRADC_MISC_DEFAULT 0x00000000

#define ADC_250HZ_RATE   0x00
#define ADC_125HZ_RATE   0x01
#define ADC_62_5HZ_RATE  0x02
#define ADC_32_25HZ_RATE 0x03

#define ADC_VOLT_REF_19 0x00
#define ADC_VOLT_REF_18 0x01
#define ADC_VOLT_REF_17 0x02
#define ADC_VOLT_REF_16 0x03

int adc_initialized = 0;
int adc_enabled = 0;

static volatile uint32_t *adc_map;
unsigned volatile page_size, page_offset;

void short_wait(void)
{
    int i;

    for (i=0; i<150; i++) {    // wait 150 cycles
        asm volatile("nop");
    }
}

int initialize_adc(void)
{
	int fd;
	uint8_t *adc_mem;
	unsigned adc_addr = LRADC;
	unsigned page_addr;

	page_size=sysconf(_SC_PAGESIZE);
	
	if (adc_initialized) {
        return 1;
    }
	
	/* Open /dev/mem file */
	fd = open ("/dev/mem", O_RDWR|O_SYNC);
	if (fd < 1) {
		return -1;
	}

	/* mmap the device into memory */
	page_addr = (adc_addr & (~(page_size-1)));
	page_offset = adc_addr - page_addr;
	
	if ((adc_mem = malloc(page_size + (page_size-1))) == NULL)
        return -2;
	
	if ((uint32_t)adc_mem % page_size)
        adc_mem += page_size - ((uint32_t)adc_mem % page_size);
	
	adc_map = (uint32_t*)mmap((void *)adc_mem, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, page_addr);
	
	short_wait();
	
	/* reset the adc */
	adc_reset();
	
    adc_initialized = 1;
    
    short_wait();
    
    /* enable the adc */
    enable(1);

    return 0;
}

void enable(int enable)
{
    if (enable) {
        *(adc_map + page_offset + CTRL_OFFSET) |= 1;
        adc_enabled = 1;
    } else {
        *(adc_map + page_offset + CTRL_OFFSET) &= ~1;
        adc_enabled = 0;
    }
}

void set_hold_enable(int enable)
{
    int shift = 6;
    if (enable) {
        *(adc_map + page_offset + CTRL_OFFSET) |= (1 << shift);
    } else {
        *(adc_map + page_offset + CTRL_OFFSET) &= ~(1 << shift);
    }
}

int set_channel(int chan)
{
    int shift = 22;
    if (chan > 2) {
        return -1;
    } else {
        *(adc_map + page_offset + CTRL_OFFSET) |= (chan << shift);
    }
    return 0;
}

int set_key_mode(int mode)
{
    int shift = 12;
    if (mode < 0 || mode > 2) {
        return -1;
    } else {
        *(adc_map + page_offset + CTRL_OFFSET) |= (mode << shift);
    }
    return 0;
}

int set_continue_time(int time)
{
    int shift = 16;
    if (time < 0 || time > 15) {
        return -1;
    } else {
        *(adc_map + page_offset + CTRL_OFFSET) |= (time << shift);
    }
    return 0;
}

int set_level_ab_count(int count)
{
    int shift = 8;
    if (count < 0 || count > 15) {
        return -1;
    } else {
        *(adc_map + page_offset + CTRL_OFFSET) |= (count << shift);
    }
    return 0;
}

int set_first_concert_delay(int time)
{
    int shift = 24;
    if (time < 0 || time > 255) {
        return -1;
    } else {
        *(adc_map + page_offset + CTRL_OFFSET) |= (time << shift);
    }
    return 0;
}

int set_sample_rate(int rate)
{
    int shift = 2;
    if (rate < 0 || rate > 3) {
        return -1;
    } else {
        *(adc_map + page_offset + CTRL_OFFSET) |= (rate << shift);
    }
    return 0;
}

int set_adc_volt_ref(int ref)
{
    int shift = 4;
    if (ref < 0 || ref > 3) {
        return -1;
    } else {
        *(adc_map + page_offset + CTRL_OFFSET) |= (ref << shift);
    }
    return 0;
}

int read_value(int chan, int *value)
{
    int offset;
    
    if (chan == 0) {
        offset = DATA0_OFFSET;
    } else if (chan == 1) {
        offset = DATA1_OFFSET;
    } else {
        return -1;
    }

    if (adc_enabled) {
        value = ((unsigned *)(adc_map + page_offset + offset));
    } else {
        return -1;
    }

    return 0;
}

void adc_reset(void)
{
    /* write the default values ot the 5 registers */
	*((unsigned *)(adc_map + page_offset + CTRL_OFFSET)) = LRADC_CTRL_DEFAULT;
    *((unsigned *)(adc_map + page_offset + INTC_OFFSET)) = LRADC_MISC_DEFAULT;
    *((unsigned *)(adc_map + page_offset + INTS_OFFSET)) = LRADC_MISC_DEFAULT;
    *((unsigned *)(adc_map + page_offset + DATA0_OFFSET)) = LRADC_MISC_DEFAULT;
    *((unsigned *)(adc_map + page_offset + DATA1_OFFSET)) = LRADC_MISC_DEFAULT;
}

int adc_setup(void)
{
    return initialize_adc();
}

void adc_cleanup(void)
{
    enable(0);
    munmap((void*)adc_map, page_size);
}
