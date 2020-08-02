#include "module_picture_manager.h"
#include "module_picture_plugin_format_jpeg.h"

#define PLUGIN_NAME "JPEG"

static int isJPEG(p_format_jpeg var)
{
	int ret;
	jpeg_stdio_src(&var->T_decompress, var->jpeg_fp);
	ret = fclose();
	if (ret)
	{
		printf("ERROR : fclose failed!\n");
	}
	ret = jpeg_read_header(&var->T_decompress, TRUE);
	if (ret == JPEG_HEADER_OK) return 0;
	else 
	{	
		printf("ERROR : It is not a jpeg file!\n");
		return -1;
	}
}

static int jpeg_open(p_format_jpeg var, char *filename,char *mode)
{
	var->jpeg_fp = fopen(filename,mode);
    if (var->jpeg_fp = NULL) 
	{  
        printf("ERROR : can't open %s\n", filename);  
        return -1;
    }
	else return 0;
}

static int jpeg_close(p_format_jpeg var)
{
	fclose(var->jpeg_fp);
}

static void jpeg_release(p_format_jpeg var)
{
	jpeg_destroy_decompress(&var->T_decompress);
}

static int jpeg_init(p_format_jpeg var)
{
	var->jpeg_fp = NULL;
	var->file_data_Size = 0;
	/* 分配和初始化一个jpeg_decompress_struct结构体 */
    var->T_decompress.err = jpeg_std_error(&var->T_err_mgr);
	jpeg_create_decompress(&var->T_decompress);
	/* 默认尺寸为原尺寸 */  
	var->T_decompress.scale_num 	= 1;  
	var->T_decompress.scale_denom 	= 1;
	return 0;
}

static int jpeg_get_RGBdata(p_format_jpeg var,p_picture_info info, unsigned int *data_RGB)
{
	int rowSize,i,j;
	unsigned char *buffer;  
	unsigned char *help;//辅助拷贝变量
	unsigned char *tmp;
	jpeg_start_decompress(&var->T_decompress);  
	rowSize = var->T_decompress.output_width * var->T_decompress.output_components;
	buffer = malloc(rowSize);
	var->file_data_Size = rowSize *var->T_decompress.output_height;
	tmp = malloc(var->file_data_Size);
	help = tmp;
	while (var->T_decompress.output_scanline < var->T_decompress.output_height)   
    {  
        /* 调用jpeg_read_scanlines得到的时候会存到buffer中 */  
        jpeg_read_scanlines(&var->T_decompress, &buffer, 1);  
        /* 将数据一行行读到缓冲区中 */  
        memcpy(help,buffer,rowSize);  
        help = help + rowSize; 
    }
	switch (info->bpp)
	{
		case 24:
		{	
			for (i = 0, j = 0; i < info->data_len ; i++, j = j+3)
			{
				data_RGB[i] = (tmp[j]<<(8*2)) + (tmp[j+1]<<(8*1)) + (tmp[j+2]<<(8*0));
			}
			break;
		}
		default: 
		{
			printf("ERROR : Don't surpport %d bpp",info->bpp);
			break;
		}
	}
	free(buffer);
	free(tmp);
	jpeg_finish_decompress(&var->T_decompress);
	return 0;
}

static int jpeg_get_info(p_format_jpeg var, p_picture_info info)
{
	
	jpeg_read_header(&var->T_decompress, TRUE);
	info->resX 		= var->T_decompress.output_width;
	info->resY 		= var->T_decompress.output_height;
	info->data_len 	= info->resX * info->resY;
	info->bpp		= var->T_decompress.output_components*8;	
	return 0;
}

picture_format_ops jpeg_ops = {
	.name 			= PLUGIN_NAME,
	.init 			= jpeg_init,
	.release 		= jpeg_release,
	.formatCorrect 	= isJPEG,
	.getRGBdata		= jpeg_get_RGBdata,
	.getInfo 		= jpeg_get_info,
	.open 			= jpeg_open,
	.close  		= jpeg_close
};

int picture_plugin_register_format_jpeg(void)
{
	int err;
	err = picture_plugin_register(&jpeg_ops);
	return err;
}

void picture_plugin_unregister_format_jpeg(void)
{
	picture_plugin_unregister(&jpeg_ops);
}