#ifndef __MODULE_PICTURE_FORMAT_BMP_H__
#define __MODULE_PICTURE_FORMAT_BMP_H__

#include <sys/stat.h>
#include <sys/mman.h>



#pragma pack(push) /* 将当前pack设置压栈保存 */
#pragma pack(1)    /* 必须在结构体定义之前使用,这是为了让结构体中各成员按1字节对齐 */
typedef struct __bmp_file_header
{
    unsigned short  Type;             /* 表明它是BMP格式的文件，内容固定为0x42,0x4D，即ASCII字符中的“B”“M” */
    unsigned int    Size;             /* BMP文件的大小，单位为字节 */
    unsigned short  Reserved[2];      /* 保留 值为0 */
    unsigned int    OffBits;          /* 位图文件头+位图信息头+调色板 */
}bmp_file_header,*p_bmp_file_header;

typedef struct __bmp_info_header
{
    unsigned int    Size;             /* 整个位图信息头结构体的大小 */
    unsigned int    Width;            /* 图像宽度，单位为像素 */
    unsigned int    Height;           /* 图像高度，单位为像素。此外，这个数的正负可以判断图像是正向还是倒向的，若为正，则表示是正向；若为负，则表示反向。 */
    unsigned short  Planes;           /* 颜色平面书，其值总为1 */
    unsigned short  BitCount;         /* 即1个像素用多少位的数据来表示，其值可能为1，4，8，16，24，32。 */
    unsigned int    Compression;      /* 数据的压缩类型 */
    unsigned int    SizeImage;        /* 图像数据的大小，单位为字节 */
    unsigned int    XPelsPerMeter;    /* 水平分辨率，单位是像素/米 */
    unsigned int    YPelsPerMeter;    /* 垂直分辨率，单位是像素/米 */
    unsigned int    ClrUsed;          /* 调色板中的颜色索引数 */
    unsigned int    ClrImportant;     /* 说明有对图像有重要影响的颜色索引的数目，若为0，表示都重要 */
}bmp_info_header,*p_bmp_info_header;
#pragma pack(pop) /* 恢复先前的pack设置 */

typedef struct __format_bmp
{
    FILE *bmp_fp;
    p_bmp_file_header PT_fileHeader;
    p_bmp_info_header PT_infoHeader;
    char *memMap_bmp;
    unsigned int file_offset;
    unsigned int mmapSize;
}format_bmp,*p_format_bmp;

int picture_plugin_register_format_bmp(void);
void picture_plugin_unregister_format_bmp(void);

#endif