#ifndef __MODULE_PICTURE_FORMAT_JPEG_H__
#define __MODULE_PICTURE_FORMAT_JPEG_H__

#include "jpeglib.h"

typedef struct __format_jpeg
{
    FILE *jpeg_fp;
    struct jpeg_decompress_struct T_decompress;
    struct jpeg_error_mgr T_err_mgr;
    unsigned int file_data_Size; 
}format_jpeg,*p_format_jpeg;

int picture_plugin_register_format_jpeg(void);
void picture_plugin_unregister_format_jpeg(void);

#endif