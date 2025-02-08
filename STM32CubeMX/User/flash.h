
#ifndef __FLASH_H
#define __FLASH_H
#include "main.h"

// 64 KB 的 Flash 存储器可以划分为 64 页（64 KB / 1 KB = 64 页）
#define PAGE_SIZE 1024                // 每页大小 (1 KB)
#define TOTAL_PAGES 64                // 总页数
#define FLASH_BASE_ADDRESS 0x08000000 // Flash 存储器起始地址
#define PAGE_ADDRESS(page) (FLASH_BASE_ADDRESS + (page * PAGE_SIZE))

void Flash_Write(uint32_t addr, uint32_t *Data, uint32_t length);
void Flash_Read(uint32_t address, uint32_t *data, uint16_t length);
void test_flash();
#endif