#ifndef __MODULE_PICTUREF_FORMAT_PNG_H__
#define __MODULE_PICTUREF_FORMAT_PNG_H__

#include <png.h>
typedef struct __format_png
{
    FILE *png_fp;
    png_structp PT_png;     //png结构体指针
    png_infop PT_pngInfo;   //png信息结构体指针
    unsigned char *rawData; //原始rgba数据
    int rawSize;            //原始rgba数据大小
    int channels;           //原始数据通道数
}format_png,*p_format_png;

int picture_plugin_register_format_png(void);
void picture_plugin_unregister_format_png(void);

#endif