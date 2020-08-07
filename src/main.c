#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <wchar.h>
#include <math.h>
#include <stdlib.h>

#include "module_display_manager.h"
#include "module_display_plugins.h"
#include "module_font_manager.h"
#include "module_font_plugins.h"
#include "module_text.h"
#include "module_picture_manager.h"
#include "module_picture_plugins.h"

#define FONT_COLOR	0x00ffffff

#define BUF_SIZE (1024*4)

#define TESTFILE_FONT	"./msyh.ttc"
#define TESTFILE_TEXT 	"./test.dat"
#define TEST_FONTSIZE 	20
#define TESTFILE_BMP 	"./test.bmp"
#define TESTFILE_JPEG 	"./test.jpeg"
#define TESTFILE_PNG 	"./test.png"

#define CMD_NULL 	0
#define CMD_EXIT 	1
#define CMD_TEST 	2

#define TEST_FONT 		10
#define TEST_PICTURE	11

#define TEST_BMP 	20
#define TEST_JPEG 	21
#define TEST_PNG 	22

static int set_blankinterval(int timeval)
{
	int fd;
	char str[20];
	if ((fd = open("/dev/tty0", O_RDWR)) < 0) 
	{
	    perror("open");
	    return -1;
	}
	sprintf(str, "\033[9;%d]", timeval);
	write(fd, str, strlen(str));
	close(fd);
	return 0;
}

static int set_cursor(int val)
{
	int fd;
	char str[20];
	switch (val)
	{
		case 0:		sprintf(str,"\033[?25l");	break;
		case 1:		sprintf(str,"\033[?25h");	break;
		default:	return -1;
	}
	if ((fd = open("/dev/tty0", O_RDWR)) < 0) 
	{
	    perror("open");
	    return -1;
	}
	write(fd, str, strlen(str));
	close(fd);
	return 0;
}

int font_test(void)
{
	char *buf;
	wchar_t *unicode_array;
	FILE *fp;
	size_t i,len;
	int err;
	buf = malloc(BUF_SIZE);
	if(buf == NULL)
	{
		printf("ERROR : buf malloc failed!\n");
		return -1;
	}
	memset(buf, 0, BUF_SIZE);				
	fp = fopen(TESTFILE_TEXT,"rb");
	if(fp == NULL)
	{
		printf("ERROR : fopen failed!\n");
		return -1;
	}
	fscanf(fp,"%[^\n]",buf);
	len = strlen(buf);
	for (i = 0; i < len; i++)
	{
		printf("buf[%d] = %02x\n",i,buf[i]);
	}
	unicode_array = malloc( (len + 1)*sizeof(wchar_t) );
	if(unicode_array == NULL)
	{
		return -1;
	}
	memset(unicode_array, 0, (len + 1)*4 );
	
	err = utf16_text_process(buf,unicode_array,len);
	if(err < 0)
	{
		printf("ERROR : utf16_text_process is failed!\n");
		return -1;
	}
	len = wcslen(unicode_array);
	for (i = 0; i < len; i++)
	{
		printf("unicode_array[%d] = %08x\n",i,unicode_array[i]);
	}
	err = font_putString(100,100,FONT_COLOR,unicode_array);
	if(err)
	{
		printf("ERROR : FT_putString_LCD is error!\n");
		return -1;
	}
	
	free(buf);
	free(unicode_array);
	return 0;
}

int picture_test(void)
{
	char *cmd;
	unsigned int *rgb_data;
	unsigned int *rgb_data_zoom;
	int ret,run,cmd_code;
	float zoom_times;
	picture_info info,info_zoom;
	run = 1;
	void *PT_format;
	while (run)	
	{	
		cmd  = malloc(BUF_SIZE);
		memset(cmd,	 0, BUF_SIZE);
		printf("Test command list:\n");
		printf("bmp 	- bmp test.\n");
		printf("jpeg	- jpeg test.\n");
		printf("png 	- png test.\n");
		printf("exit 	- exit the test.\n");
		printf("Please enter command:");
		scanf("%s",cmd);
		if 		( !(strcmp(cmd,"bmp")) )	cmd_code = TEST_BMP;
		else if	( !(strcmp(cmd,"jpeg")) )	cmd_code = TEST_JPEG;
		else if	( !(strcmp(cmd,"png")) )	cmd_code = TEST_PNG;
		else if	( !(strcmp(cmd,"exit")) )	cmd_code = CMD_EXIT;
		else cmd_code = CMD_NULL;
		free(cmd);

		switch(cmd_code)
		{			
			case CMD_NULL:
			{
				printf("NULL\n");
				break;
			}
			case CMD_EXIT:
			{
				printf("EXIT PICTURE TEST\n");
				run = 0;
				picture_close(PT_format);
				picture_plugin_release(PT_format);
				break;
			}
			case TEST_BMP:
			{
				display_fill(0);
				format_bmp PT_test;
				printf("BMP TEST\n");
				ret = picture_plugin_format_select("BMP");
				if(ret) return -1;
				ret = picture_plugin_init(&PT_test);
				if(ret) return -1;
				ret = picture_open(&PT_test, TESTFILE_BMP,"rb");
				if(ret) return -1;
				ret = picture_formatCorrect(&PT_test);
				if(ret) return -1;
				ret = picture_getInfo(&PT_test,&info);
				if(ret) return -1;
				rgb_data = malloc(sizeof(unsigned int)*info.data_len);
				if(rgb_data == NULL) return -1;
				picture_decode(&PT_test,&info,rgb_data);
				printf("plaese input zoom times:");
				scanf("%f",&zoom_times);
				info_zoom.bpp 		= info.bpp;
				info_zoom.resX 		= (unsigned int)(zoom_times*info.resX);
				info_zoom.resY 		= (unsigned int)(zoom_times*info.resY);
				info_zoom.data_len 	= info_zoom.resX*info_zoom.resY;
				rgb_data_zoom = malloc(sizeof(unsigned int)*info_zoom.data_len);
				picture_zoom(&info,&info_zoom,rgb_data,rgb_data_zoom);
				picture_display(10,10,rgb_data_zoom,&info_zoom);
				free(rgb_data);
				free(rgb_data_zoom);
				PT_format = &PT_test;
				break;
			}  
			case TEST_JPEG:
			{
				display_fill(0);
				format_jpeg PT_test;
				printf("JPEG TEST\n");
				ret = picture_plugin_format_select("JPEG");
				if(ret) return -1;
				ret = picture_plugin_init(&PT_test);
				if(ret) return -1;
				ret = picture_open(&PT_test, TESTFILE_JPEG,"rb");
				if(ret) return -1;
				ret = picture_formatCorrect(&PT_test);
				if(ret) return -1;
				ret = picture_getInfo(&PT_test,&info);
				if(ret) return -1;
				rgb_data =  malloc(sizeof(unsigned int)*info.data_len);
				if(rgb_data == NULL) return -1;
				picture_decode(&PT_test,&info,rgb_data);
				picture_display(10,10,rgb_data,&info);
				free(rgb_data);
				PT_format = &PT_test;
				break;
			}
			case TEST_PNG:
			{
				display_fill(0);
				format_png PT_test;
				printf("PNG TEST\n");
				ret = picture_plugin_format_select("PNG");
				if(ret) return -1;
				ret = picture_plugin_init(&PT_test);
				if(ret) return -1;
				ret = picture_open(&PT_test, TESTFILE_PNG,"rb");
				if(ret) return -1;
				ret = picture_formatCorrect(&PT_test);
				if(ret) return -1;
				ret = picture_getInfo(&PT_test,&info);
				if(ret) return -1;
				rgb_data = malloc(sizeof(unsigned int)*info.data_len);
				if(rgb_data == NULL) return -1;
				picture_decode(&PT_test,&info,rgb_data);
				picture_display(10,10,rgb_data,&info);
				free(rgb_data);
				PT_format = &PT_test;
				break;
			}
			default: return -1;
		}
	}
	return 0;
}

int test(void)
{
	char *cmd;
	int ret,run,cmd_code;
	while(run)
	{	
		cmd  = malloc(BUF_SIZE);
		if(cmd == NULL)
		{
			printf("ERROR : malloc failed!\n");
			return -1;
		}
		memset(cmd,	 0, BUF_SIZE);
		printf("Test command list:\n");
		printf("font 	- font test.\n");
		printf("picture - picture test.\n");
		printf("exit 	- exit test.\n");
		printf("Please enter command:");
		scanf("%s",cmd);
		if		( !(strcmp(cmd,"font")) )		cmd_code = TEST_FONT;
		else if	( !(strcmp(cmd,"picture")) )	cmd_code = TEST_PICTURE;
		else if	( !(strcmp(cmd,"exit")) )		cmd_code = CMD_EXIT;
		else cmd_code = CMD_NULL;
		free(cmd);
		switch(cmd_code)
		{			
			case CMD_NULL:
			{
				printf("NULL\n");
				break;
			}
			case CMD_EXIT:
			{
				printf("EXIT TEST\n");
				run = 0;
				break;
			}
			case TEST_FONT:
			{
				printf("FONT TEST\n");
				ret = font_test();
				if (ret) return ret;
				break;
			}  
			case TEST_PICTURE:
			{
				printf("PICTURE TEST\n");
				ret = picture_test();
				if (ret) return ret;
				break;
			}  
			default:
			{
				printf("ERROR : cmd error\n");
				run = 0;
				break;
			}
		}
	}
	return 0;
}

void process_main(void)
{
	char *cmd;
	int run,cmd_code,err;
	err = 0;
	run = 1;
	while(run)
	{	
		cmd  = malloc(BUF_SIZE);
		memset(cmd,	 0, BUF_SIZE);
		printf("Command list:\n");
		printf("test - feature test.\n");
		printf("exit - exit the test program.\n");
		printf("Please enter command:");
		scanf("%s",cmd);
		if 		( !(strcmp(cmd,"exit")) )	cmd_code = CMD_EXIT;
		else if ( !(strcmp(cmd,"test")) )	cmd_code = CMD_TEST;
		else cmd_code = CMD_NULL;
		free(cmd);
		switch(cmd_code)
		{
			case CMD_NULL:
			{
				printf("NULL\n");
				break;
			}
			case CMD_EXIT:
			{
				printf("EXIT\n");
				run = 0;
				break;
			}  
			case CMD_TEST:
			{
				err = test();
				if(err) run = 0;
				
				break;
			}
			default:
			{
				printf("ERROR : cmd is error!\n");
				run = 0;
				break;
			}
		}
	}
}

int main (int argc,char **argv)
{
/*     if (argc != 2)
    {
        printf("Usage : %s <font>\n",argv[0]);
        return -1;
    } */
	display_var parmas;
	font_var font_setting;
	picture_var picture_setting;
	int err,font_size;
	/* 设置LCD休眠时间 */
    err = set_blankinterval(0);
	if(err) 
	{
		printf("set_blankinterval is failed!\n");
		return err;	
	}
	err = set_cursor(0);
	if(err) 
	{
		printf("set_cursor is failed!\n");
		return err;	
	}
	/* LCD module初始化 得到lcd参数以及画点函数 */
	err = display_module_init();
	if (err)
	{
		printf("display_module_init is failed!\n");
		return err;
	}
	err = display_plugin_select("lcd");
	if (err)
	{
		printf("display_plugin_select is failed!\n");
		return err;
	}
	err = display_plugin_init();
	if (err)
	{
		printf("display_plugin_init is failed!\n");
		return err;
	}
	display_get_var(&parmas);

	/* font module初始化 传入画点函数以及lcd的参数*/
	/* printf("Please setup font size:");
	scanf("%d",&font_size); */
	font_size = TEST_FONTSIZE;
	font_setting.filename = TESTFILE_FONT;
	font_setting.fontsize_width	= font_size;
	font_setting.fontsize_height = font_size;
	font_setting.border_x = parmas.res_x;
	font_setting.border_y = parmas.res_y;
	font_setting.draw_dot = display_put_pixel;
	err = font_module_init();
	if (err)
	{
		printf("font_module_init is failed!\n");
		return err;
	}
	err = font_plugin_select("FreeType");
	if (err)
	{
		printf("font_plugin_select is failed!\n");
		return err;
	}
	err = font_plugin_init(&font_setting);
	if (err)
	{
		printf("font_plugin_init is failed!\n");
		return err;
	}
	
	/* picture module初始化 传入画点函数以及lcd的参数*/
	picture_setting.border_x = parmas.res_x;
	picture_setting.border_y = parmas.res_y;
	picture_setting.draw_dot = display_put_pixel;
	picture_setting.zoom_mode = 0;
	picture_module_init(&picture_setting);
	/* 主线程 */
	process_main();

	set_cursor(1);
	set_blankinterval(10*60);
	/* 移除各个模块 */
	picture_module_remove();
	font_module_remove();
	display_module_remove();
	
	return 0;
}


