#include "module_picture_manager.h"
#include "module_picture_plugin_format_png.h"

#define PLUGIN_NAME "PNG"

static int isPNG(void *format_var)
{
	int err;
	p_format_png var;
	var = (p_format_png)format_var;
	unsigned char tmp[8];
	err = fread(tmp,1,8,var->png_fp);
	if (err != 8)
	{
		printf("ERROR : fread failed!\n");
		return -1;
	}
	err = png_sig_cmp((png_bytep)&tmp, 0, 8);
	if (err)
	{
		printf("ERROR : It is not png file!\n");
		return -1;
	}
	return 0;
}

static int png_open(void *format_var, char *filename,char *mode)
{
	p_format_png var;
	var = (p_format_png)format_var;
	var->png_fp = fopen(filename,mode);
	if (var->png_fp == NULL) 
	{
        printf("ERROR : can't open %s\n", filename);  
        return -1;  
    }
	else return 0;
}

static int png_close(void *format_var)
{
	int err;
	p_format_png var;
	var = (p_format_png)format_var;
	err = fclose(var->png_fp);
	if(err)
	{
		printf("ERROR : png_close failed!\n");
		perror("fclose");
		return err;
	}
	return 0;
}

static void png_release(void *format_var)
{
	p_format_png var;
	var = (p_format_png)format_var;
	if(var->png_fp != NULL) png_close(format_var);
	png_destroy_read_struct(&var->PT_png, &var->PT_pngInfo, 0);
}

static int png_init(void *format_var)
{
	p_format_png var;
	var = (p_format_png)format_var;
	var->PT_png = NULL;
	var->PT_pngInfo = NULL;
	var->png_fp = NULL;
	var->rawData = NULL;
	var->rawSize = 0;
	var->channels = 0;

	var->PT_png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (var->PT_png == NULL)
	{
		printf("ERROR : png_create_read_struct failed!\n");
		return -1;
	}
	setjmp( png_jmpbuf(var->PT_png) );
	fseek(var->png_fp, 0, SEEK_SET);
	var->PT_pngInfo = png_create_info_struct(var->PT_png);
	if (var->PT_pngInfo == NULL)
	{
		printf("ERROR : png_create_info_struct failed!\n");
		return -1;
	}
	return 0;
}

static int png_get_info(void *format_var, p_picture_info info)
{
	p_format_png var;
	var = (p_format_png)format_var;
	png_read_png(var->PT_png, var->PT_pngInfo, PNG_TRANSFORM_EXPAND, 0); 
	var->channels 	= png_get_channels(var->PT_png, var->PT_pngInfo);
	info->resX 		= png_get_image_width(var->PT_png, var->PT_pngInfo);
	info->resY 		= png_get_image_height(var->PT_png, var->PT_pngInfo);
	info->data_len 	= info->resX * info->resY;
	info->bpp 		= var->channels * 8;
	return 0;
}

static int png_get_RGBdata(void *format_var,p_picture_info info, unsigned int *data)
{
	int i,j,k,iPos,rawDataSize;
	png_bytepp tmp;
	iPos = 0;
	p_format_png var;
	var = (p_format_png)format_var;
	/* 将info_ptr中的图像数据读取出来 */
	tmp = png_get_rows(var->PT_png, var->PT_pngInfo); //也可以分别每一行获取png_get_rowbytes();
	if (var->channels == 4) 
	{ //判断是24位还是32位
		rawDataSize= info->resX * info->resY *4; //申请内存先计算空间	
		var->rawData= (unsigned char*)malloc(rawDataSize);
		if (NULL == var->rawData) 
		{
			printf("malloc rgba faile ...\n");
			png_destroy_read_struct(&var->PT_png, &var->PT_pngInfo, 0);
			png_close(format_var);
			return -1;
		}
		/* 从tmp里读出实际的RGBA数据出来 
		 * 源数据为ABGR格式*/
		for (i = 0; i < info->resY; i++)
		{ 
			for (j = 0; j < info->resX * 4; j += 4) 
			{
				var->rawData[iPos++] = tmp[i][j + 3];	/* R */
				var->rawData[iPos++] = tmp[i][j + 2];	/* G */
				var->rawData[iPos++] = tmp[i][j + 1];	/* B */
				var->rawData[iPos++] = tmp[i][j + 0];	/* A */
				/* 将得到的RGBA转换为RGB888格式 */
				data[k] = (var->rawData[iPos - 3]<<(8*2)) + (var->rawData[iPos - 2]<<(8*1)) + (var->rawData[iPos - 1]<<(8*0));
				k++;
			}
		}
	}
	else if (var->channels == 3 ) 
	{ //判断颜色深度是24位还是32位
		rawDataSize= info->resX * info->resY*3; //申请内存先计算空间	
		var->rawData = (unsigned char*)malloc(rawDataSize);
		if (NULL == var->rawData) 
		{
			printf("malloc rgba faile ...\n");
			png_destroy_read_struct(&var->PT_png, &var->PT_pngInfo, 0);
			png_close(format_var);
			return -1;
		}
		/* 从tmp里读出实际的RGB数据
		  * 源数据为BGR格式*/
		for (i = 0; i < info->resY; i ++) 
		{
			for (j = 0; j < info->resX*3; j += 3) 
			{
				var->rawData[iPos++] = tmp[i][j+2];	/* R */
				var->rawData[iPos++] = tmp[i][j+1];	/* G */
				var->rawData[iPos++] = tmp[i][j+0];	/* B */
				data[k] = (var->rawData[iPos - 2]<<(8*2)) + (var->rawData[iPos - 1]<<(8*1)) + (var->rawData[iPos]<<(8*0));
				k++;
			}
		}
	}
	return 0;
}

picture_format_ops png_ops = {
	.name 			= PLUGIN_NAME,
	.init 			= png_init,
	.formatCorrect 	= isPNG,
	.getInfo 		= png_get_info,
	.getRGBdata 	= png_get_RGBdata,
	.release 		= png_release,
	.open 			= png_open,
	.close 			= png_close
};

int picture_plugin_register_format_png(void)
{	
	int err;
	err = picture_plugin_register(&png_ops);
	return err;
}

void picture_plugin_unregister_format_png(void)
{	
	picture_plugin_unregister(&png_ops);
}
