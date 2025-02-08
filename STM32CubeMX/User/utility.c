#include <stdlib.h> // 用于 malloc 和 free
#include <string.h> // 用于 memset
#include <stdint.h> // 用于 uint8_t
#include "utility.h"

void AppendString(char *str, uint8_t *buf, int buf_len, uint8_t *output, int *output_len)
{
    int len = strlen(str);
    strcpy((char *)output, str);
    memcpy(output + len, buf, buf_len);
    *output_len = len + buf_len;
}

void AppendByteArray(uint8_t *bigArray, size_t bigArraySize, uint8_t *smallArray, size_t smallArraySize)
{
    // 找到大数组的当前有效长度
    size_t offset = strlen((const char *)bigArray); // 假设大数组是以字符串形式存储的
    size_t bytesToCopy = smallArraySize;

    if (offset + smallArraySize >= bigArraySize)
    {
        bytesToCopy = bigArraySize - offset;
    }

    memcpy(bigArray + offset, smallArray, bytesToCopy); // 追加小数组
}

void CopyArray(uint8_t *dest, const uint8_t *src, size_t length)
{
    if (dest == NULL || src == NULL)
    {
        // 处理空指针情况
        return;
    }

    memcpy(dest, src, length);
}

void ClearArray(uint8_t *array, size_t size)
{
    memset(array, 0x00, size); // 将数组所有元素设置为 0
}

void APP_Delay(uint32_t xus)
{
    // 计算循环次数
    volatile uint32_t count;

    while (xus--)
    {
        // 72MHz 约 72 个循环大约 1 微秒
        count = 72; // 具体的计数值可能需要根据编译器优化进行调整
        while (count--)
            ;
    }
}
