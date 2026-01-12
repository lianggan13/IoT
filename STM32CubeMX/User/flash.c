
#include "flash.h"
#include <stdio.h>
#include <string.h>

void flash_read(uint32_t address, uint8_t *data, uint32_t lengthBytes)
{
    if ((data == NULL) || (lengthBytes == 0))
    {
        return;
    }

    memcpy(data, (const void *)address, lengthBytes);
}

void flash_readString(uint32_t addr, char *out, size_t outSize)
{
    if (!out || outSize == 0)
        return;

    flash_read(addr, (uint8_t *)out, (uint32_t)outSize);
    out[outSize - 1] = '\0';

    // 擦除态一般是全 0xFF：按“空字符串”处理
    if ((uint8_t)out[0] == 0xFF)
        out[0] = '\0';

    // 可选：遇到 0xFF 提前截断，避免把擦除态尾部当成字符串内容
    // for (size_t i = 0; i + 1 < outSize; i++)
    //     if ((uint8_t)out[i] == 0xFF)
    //     {
    //         out[i] = '\0';
    //         break;
    //     }
}

void flash_write(uint32_t addr, const uint8_t *data, uint32_t lengthBytes)
{
    if ((data == NULL) || (lengthBytes == 0))
    {
        return;
    }

    HAL_FLASH_Unlock();

    uint32_t startPageAddr = addr - (addr % PAGE_SIZE);
    uint32_t endAddr = addr + lengthBytes - 1;
    uint32_t endPageAddr = endAddr - (endAddr % PAGE_SIZE);
    uint32_t nbPages = ((endPageAddr - startPageAddr) / PAGE_SIZE) + 1;

    FLASH_EraseInitTypeDef FlashSet;
    FlashSet.TypeErase = FLASH_TYPEERASE_PAGES;
    FlashSet.PageAddress = startPageAddr;
    FlashSet.NbPages = nbPages;
    uint32_t PageError = 0;
    if (HAL_FLASHEx_Erase(&FlashSet, &PageError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        printf("flash erase failed\r\n");
        return;
    }

    HAL_StatusTypeDef flash_status = HAL_OK;
    for (uint32_t off = 0; off < lengthBytes; off += 4)
    {
        uint32_t word = 0xFFFFFFFFu;
        uint32_t chunk = (lengthBytes - off >= 4) ? 4 : (lengthBytes - off);
        memcpy(&word, data + off, chunk);

        flash_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + off, word);
        if (flash_status != HAL_OK)
        {
            break;
        }
    }

    if (flash_status != HAL_OK)
    {
        printf("flash write failed\r\n");
    }

    HAL_FLASH_Lock();
}

void print_page_addresses()
{
    for (int page = 0; page < TOTAL_PAGES; page++)
    {
        uint32_t address = PAGE_ADDRESS(page);
        printf("Page %d starts at address: 0x%08X\n", page, address);
    }
}

void test_flash()
{
    const char *str_wifi = "AT+CWJAP=\"LG13_TPLink_2G\",\"G15608212470*\"";

    flash_write(PAGE_ADDRESS(31), (const uint8_t *)str_wifi, (uint32_t)strlen(str_wifi) + 1);

    static char wifi[128];

    flash_read(PAGE_ADDRESS(31), (uint8_t *)wifi, sizeof(wifi));
    wifi[sizeof(wifi) - 1] = '\0';

    printf("%s\n", wifi);

    const char *str_tcp = "AT+CIPSTART=\"TCP\",\"192.168.0.107\",1918";

    flash_write(PAGE_ADDRESS(32), (const uint8_t *)str_tcp, (uint32_t)strlen(str_tcp) + 1);

    static char tcp[128];

    flash_read(PAGE_ADDRESS(32), (uint8_t *)tcp, sizeof(tcp));
    tcp[sizeof(tcp) - 1] = '\0';

    printf("%s\n", tcp);

    while (1)
    {
        /* code */
    }
}
