/*
 * Copyright (c) 2013 Yannik Li(Yanqing Li)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __TEXT_H
#define __TEXT_H
#include <arch/chip_regs.h>
#include <arch/memory.h>
#include <kernel/types.h>

//#define BOOTUP_UART_BASE (uint32_t)phys_to_virt(REG_BASE_UART0)
#define BOOTUP_UART_BASE REG_BASE_UART0
#define UART_DR	         (*(volatile unsigned char *)(BOOTUP_UART_BASE + 0x000))
#define UART_ECR         (*(volatile unsigned char *)(BOOTUP_UART_BASE + 0x004))
#define UART_LCRH	 (*(volatile unsigned char *)(BOOTUP_UART_BASE + 0x02C))
#define UART_CR	         (*(volatile unsigned char *)(BOOTUP_UART_BASE + 0x030))
#define UART_FR	         (*(volatile unsigned char *)(BOOTUP_UART_BASE + 0x018))

#define UART_FR_TXFE              0x80
#define UART_FR_RXFF              0x40
#define UART_FR_TXFF              0x20
#define UART_FR_RXFE              0x10
#define UART_FR_BUSY              0x08
#define UART_FR_TMSK              (UART_FR_TXFF + UART_FR_BUSY)

void __console_init(void);
void __puts(const char *str);
void __puts_early(const char *str);
void putchar(char c);
int getchar(void);

#endif
