
#include "flash.h"
#include <stdio.h>
#include <string.h>

void print_page_addresses()
{
    for (int page = 0; page < TOTAL_PAGES; page++)
    {
        uint32_t address = PAGE_ADDRESS(page);
        printf("Page %d starts at address: 0x%08X\n", page, address);
    }
}

void Flash_Write(uint32_t addr, uint32_t *Data, uint32_t length)
{
    uint32_t i = 0;
    HAL_StatusTypeDef flash_status;
    /*解锁FLASH*/
    HAL_FLASH_Unlock();
    /*擦除FLASH*/
    FLASH_EraseInitTypeDef FlashSet;            // 初始化FLASH_EraseInitTypeDef
    FlashSet.TypeErase = FLASH_TYPEERASE_PAGES; // 擦除方式页擦除FLASH_TYPEERASE_PAGES,块擦除FLASH_TYPEERASE_MASSERASE
    FlashSet.PageAddress = addr;                // 擦除地址
    FlashSet.NbPages = 1;                       // 擦除页数
    uint32_t PageError = 0;                     // 设置PageError
    HAL_FLASHEx_Erase(&FlashSet, &PageError);   // 调用擦除函数
    /*对FLASH烧写*/
    for (i = 0; i < length; i++)
    {
        flash_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + 4 * i, Data[i]);
    }
    if (flash_status == HAL_OK)
    {
        // printf("flash write ok\r\n");
    }
    else if (flash_status != HAL_OK)
    {
        printf("flash write failed ×\r\n");
    }
    /*上锁FLASH*/
    HAL_FLASH_Lock();
}

void Flash_Read(uint32_t address, uint32_t *data, uint16_t length)
{
    uint16_t i;
    for (i = 0; i < length; i++)
    {
        data[i] = *(__IO uint32_t *)(address + (i * 4)); // 以字为单位读取Flash
    }
}

void test_flash()
{
    const char *str_wifi = "AT+CWJAP=\"LG13_TPLink_2G\",\"G15608212470*\"";

    Flash_Write(PAGE_ADDRESS(31), (uint32_t *)str_wifi, strlen(str_wifi));

    static char wifi[128];

    Flash_Read(PAGE_ADDRESS(31), (uint32_t *)wifi, sizeof(wifi));

    printf("%s\n", wifi);

    const char *str_tcp = "AT+CIPSTART=\"TCP\",\"192.168.0.107\",1918";

    Flash_Write(PAGE_ADDRESS(32), (uint32_t *)str_tcp, strlen(str_tcp));

    static char tcp[128];

    Flash_Read(PAGE_ADDRESS(32), (uint32_t *)tcp, sizeof(tcp));

    printf("%s\n", tcp);

    while (1)
    {
        /* code */
    }
}