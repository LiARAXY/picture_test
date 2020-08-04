#include "module_picture_manager.h"
#include "module_picture_plugin_format_bmp.h"

/* BMP文件中数据以小端模式存放 */
#define PLUGIN_NAME "BMP"

static int mmapFile(void *format_var)
{
	p_format_bmp var;
	var = (p_format_bmp)format_var;
	if (var->bmp_fp == NULL)
	{
		return -1;
	}
	int fd;
	struct stat file_status;
	fd = fileno(var->bmp_fp);
	fstat(fd,&file_status);
	var->mmapSize = file_status.st_size;
	var->memMap_bmp = mmap(NULL, var->mmapSize, PROT_READ, MAP_SHARED, fd, 0);
	if(var->memMap_bmp == NULL) 
	{
		printf("ERROR : mmapFile failed!\n");
		return -1;}
	else 
	{
		var->file_offset = 0;
		return 0;
	}
}

static int dataRead(p_format_bmp var, unsigned int len,void *data)
{
	char *tmp,*target,*start;
	unsigned int i;
	tmp = malloc(len);
	if (tmp == NULL) return -1;
	memset(tmp, 0, len);
	target = (char *)data;
	start = var->memMap_bmp + var->file_offset;
	memcpy(tmp,start,len);
	for(i = 0; i < len; i++)
	{
		target[i] = tmp[i];
	}
	var->file_offset += len;
	free(tmp);
	return 0;
}

static int isBMP(void *format_var)
{
	char tmp[2];
	p_format_bmp var;
	var = (p_format_bmp)format_var;
	fread(tmp,1,2,var->bmp_fp);
	fclose(var->bmp_fp);
	/*          B                   M                 */
	if( (tmp[0] == 0x42) && (tmp[1] == 0x4d) ) return 0;
	else 
	{
		printf("ERROR : It is not bmp file!\n");
		return -1;
	}	
}

static int bmp_get_fileHeader(p_format_bmp var)
{
	int err;
	var->file_offset = 0;
	err = dataRead(var, sizeof(bmp_file_header),var->PT_fileHeader);
	if (err)
	{
		printf("ERROR : get file header failed!\n");
		return -1;
	}
	return 0;
}

static int bmp_get_infoHeader(p_format_bmp var)
{
	int err;
	var->file_offset = sizeof(bmp_file_header);
	err = dataRead(var, sizeof(bmp_info_header), var->PT_infoHeader);
	if (err)
	{
		printf("ERROR : get info header failed!\n");
		return -1;
	}
	return 0;
}

static unsigned int bmp_bpp24_get_pixel(p_format_bmp var)
{
	unsigned int ret;
	int err;
	pixel_color tmp;
	err = dataRead(var, sizeof(pixel_color), &tmp);	
	if(err) return 0xff000000;
	ret = (tmp.red<<(8*2)) + (tmp.green<<8) + tmp.blue;
	return ret;
}


static int bmp_bpp24_get_RGBdata(p_format_bmp var, p_picture_info info, unsigned int *data_RGB)
{
	unsigned int i,j,pos,tmp;
	var->file_offset = sizeof(bmp_file_header) + sizeof(bmp_info_header);
	printf("file offset = %d",var->file_offset);
	for ( i = 0; i < info->resY ; i++)
	{
		for ( j = 0; j < info->resY ; j++)
		{
			pos = (info->resY - i -1)*(info->resX) + j ;
			tmp = bmp_bpp24_get_pixel(var);
			if(tmp <0x1000000) data_RGB[pos] = tmp;
			else return -1;
		}
		var->file_offset += 2;
	}
	return 0;
}

static int bmp_get_RGBdata(void *format_var, p_picture_info info, unsigned int *data_RGB)
{
	int err;
	p_format_bmp var;
	var = (p_format_bmp)format_var;
	if(var->bmp_fp== NULL) return -1;
	switch (var->PT_infoHeader->BitCount)
	{
		//case 1:			break;
		//case 4:			break;
		//case 8:			break;
		//case 16:			break;
		case 24: err = bmp_bpp24_get_RGBdata(var, info,data_RGB); break;
		//case 32:			break;
		default: 
		{
			err = -1; 
			printf("ERROR : Don't support %d bpp!\n",var->PT_infoHeader->BitCount);
			break;
		}
	}
	return err;
}

static int bmp_close(void *format_var)
{	
	int err;
	p_format_bmp var;
	var = (p_format_bmp)format_var;
	err = munmap(var->memMap_bmp,var->mmapSize);
	if(err)
	{
		printf("ERROR : mumap failed!\n");
		return err;
	}
	err = fclose(var->bmp_fp);
	if(err)
	{
		printf("ERROR : bmp_close failed!\n");
		perror("fclose");
		return err;
	}
	free(var->PT_fileHeader);
	var->PT_fileHeader = NULL;
	free(var->PT_infoHeader);
	var->PT_infoHeader = NULL;
	return 0;
}


static void bmp_release(void *format_var)
{
	int err;
	p_format_bmp var;
	var = (p_format_bmp)format_var;
	if(var->memMap_bmp 		!= NULL) 
	{
		err = munmap(var->memMap_bmp,var->mmapSize);
		if(err) printf("ERROR : munmap failed\n");	
	}
	if(var->PT_fileHeader 	!= NULL) free(var->PT_fileHeader);
	if(var->PT_infoHeader 	!= NULL) free(var->PT_infoHeader);
	if(var->bmp_fp	  		!= NULL) bmp_close(format_var);
}

static int bmp_open(void *format_var, char *filename, char *mode)
{
	int err;
	p_format_bmp var;
	var = (p_format_bmp)format_var;
	var->bmp_fp= fopen(filename,mode);
	if(var->bmp_fp== NULL)
	{
		printf("ERROR : can't open %s",filename);
		return -1;
	}
	err = mmapFile(var);
	if(err) 
	{
		bmp_release(var);
		return -1;
	}
	return 0;
}

static int bmp_init(void *format_var)
{
	p_format_bmp var;
	var = (p_format_bmp)format_var;
	var->bmp_fp			= NULL;
	var->memMap_bmp 	= NULL;
	var->file_offset	= 0;
	var->mmapSize		= 0;
	var->PT_fileHeader = malloc(sizeof(bmp_file_header));
	if (var->PT_fileHeader == NULL)
	{
		printf("ERROR : PT_fileHeader malloc failed!\n");
		bmp_release(var);
		return -1;
	}
	var->PT_infoHeader = malloc(sizeof(bmp_info_header));
	if (var->PT_infoHeader == NULL)
	{
		printf("ERROR : PT_infoHeader malloc failed!\n");
		bmp_release(var);
		return -1;
	}
	return 0;
}

static int bmp_get_info(void *format_var,p_picture_info info)
{
	int err;
	p_format_bmp var;
	var = (p_format_bmp)format_var;
	err = bmp_get_fileHeader(var);
	if(err) return -1;
	err = bmp_get_infoHeader(var);
	if(err) return -1;
	info->resX 		= var->PT_infoHeader->Width;
	info->resY 		= var->PT_infoHeader->Height;
	info->data_len 	= info->resX * info->resY;
	info->bpp		= var->PT_infoHeader->BitCount;
	return 0;
}


picture_format_ops bmp_ops = {
	.name 			= PLUGIN_NAME,
	.init 			= bmp_init,
	.release 		= bmp_release,
	.formatCorrect 	= isBMP,
	.getRGBdata		= bmp_get_RGBdata,
	.getInfo 		= bmp_get_info,
	.open 			= bmp_open,
	.close 			= bmp_close
};

int picture_plugin_register_format_bmp(void)
{	
	int err;
	err = picture_plugin_register(&bmp_ops);
	return err;
}

void picture_plugin_unregister_format_bmp(void)
{	
	picture_plugin_unregister(&bmp_ops);
}
