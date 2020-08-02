#include "module_picture_manager.h"
#include "module_picture_plugin_format_bmp.h"

/* BMP文件中数据以小端模式存放 */
#define PLUGIN_NAME "BMP"

static int mmapFile(p_format_bmp var)
{
	if (var->fp == NULL)
	{
		return -1;
	}
	int fd;
	struct stat file_status;
	fd = fileno(var->fp);
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
	int err;
	tmp = malloc(len);
	if (tmp == NULL) return -1;
	memset(tmp, 0, len);
	target = (char *)data;
	start = var->memMap_bmp + var->file_offset;
	memcpy(tmp,var->bmp_fp,len);
	for(i = 0; i < len; i++)
	{
		target[(len-1) - i] = tmp[i];
	}
	var->file_offset += len;
	free(tmp);
	return 0;
}

static int isBMP(p_format_bmp var)
{
	char tmp[2];
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
	if (err)
	{
		printf("ERROR : get file header failed!\n");
		return -1;
	}
	dataRead(var, sizeof(bmp_file_header),var->PT_fileHeader);
	return 0;
}

static int bmp_get_infoHeader(p_format_bmp var)
{
	long offset;
	int err;
	var->file_offset = sizeof(bmp_file_header);
	if (err)
	{
		printf("ERROR : get info header failed!\n");
		return -1;
	}
	dataRead(var, sizeof(bmp_info_header), var->PT_infoHeader);
	return 0;
}

static int bmp_bpp24_get_RGBdata(p_format_bmp var, p_picture_info info, unsigned int *data_RGB)
{
	unsigned int i,j;
	var->file_offset = sizeof(bmp_file_header) + sizeof(bmp_info_header);
	for ( i = 0; i < info->resY ; i++)
	{
		for ( j = 0; j < info->resX ; j++)
		{
			dataRead(var, 3, &data_RGB[j + (i * (info->resX))]);
		}
	}
	return 0;
}

static int bmp_get_RGBdata(p_format_bmp var, unsigned int *data_RGB)
{
	if(var->bmp_fp== NULL) return -1;
	long fp_offset;
	int err;
	switch (PT_infoHeader->biBitCount)
	{
		//case 1:			break;
		//case 4:			break;
		//case 8:			break;
		//case 16:			break;
		case 24: err = bmp_bpp24_get_RGBdata(var,data_RGB); break;
		//case 32:			break;
		default: 
		{
			err = -1; 
			printf("ERROR : Don't support %d bpp!\n",var->PT_infoHeader->biBitCount);
			break;
		}
	}
	return err;
}

static int bmp_open(p_format_bmp var, char *filename, char *mode)
{
	int err;
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

static void bmp_close(p_format_bmp var)
{	
	int err;
	err = fclose(var->bmp_fp);
	if(err)
	{
		printf("ERROR : bmp_close failed!\n");
		return -1;
	}
	return 0;
}

static void bmp_release(p_format_bmp var)
{
	if(var->memMap_bmp 		!= NULL) munmap(var->memMap_bmp);
	if(var->PT_fileHeader 	!= NULL) free(var->PT_fileHeader);
	if(var->PT_infoHeader 	!= NULL) free(var->PT_infoHeader);
	if(var->bmp_fp	  		!= NULL) bmp_fclose(var->bmp_fp);
}

static int bmp_init(p_format_bmp var)
{
    int err;
	var->bmp_fp			= NULL;
	var->memMap_bmp 	= NULL;
	var->PT_fileHeader 	= NULL;
	var->PT_infoHeader 	= NULL;
	var->file_offset	= 0;
	var->mmapSize		= 0;

	var->PT_fileHeader = malloc(sizeof(bmp_file_header));
	if (var->PT_fileHeader == NULL)
	{
		bmp_release(var);
		return -1;
	}
	var->PT_infoHeader = malloc(sizeof(bmp_info_header));
	if (var->PT_infoHeader == NULL)
	{
		bmp_release(var);
		return -1;
	}
	return 0;
}

static void bmp_get_info(p_format_bmp var,p_picture_info info)
{
	info->resX 		= var->PT_infoHeader->biWidth;
	info->resY 		= var->PT_infoHeader->biHeight;
	info->data_len 	= info->resX * info->resY;
	info->bpp		= var->PT_infoHeader->biBitCount;
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
