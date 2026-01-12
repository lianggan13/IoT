
#ifndef __FLASH_H
#define __FLASH_H

#include <stdint.h>
#include "stm32f1xx_hal.h"
// #include "main.h"

// 64 KB 的 Flash 存储器可以划分为 64 页（64 KB / 1 KB = 64 页）
#define PAGE_SIZE 1024                // 每页大小 (1 KB)
#define TOTAL_PAGES 64                // 总页数
#define FLASH_BASE_ADDRESS 0x08000000 // Flash 存储器起始地址
#define PAGE_ADDRESS(page) (FLASH_BASE_ADDRESS + (page * PAGE_SIZE))

void test_flash(void);

void flash_read(uint32_t address, uint8_t *data, uint32_t lengthBytes);
void flash_read_str(uint32_t addr, char *out, size_t outSize);
void flash_write(uint32_t addr, const uint8_t *data, uint32_t lengthBytes);

#endif
