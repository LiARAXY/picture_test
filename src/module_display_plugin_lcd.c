#include "module_display_manager.h"
#include "module_display_plugin_lcd.h"

#define PLUGIN_NAME "lcd"
#define FB_PATH 	"/dev/fb0"

static int fd_fb;
static struct fb_var_screeninfo var;	/* Current var */
static int screen_size;
static unsigned char *fbmem;
static unsigned int line_width;
static unsigned int pixel_width;

static void lcd_put_pixel(int x,int y,unsigned int color)
{
    unsigned char *pen_8 = fbmem + y*line_width + x*pixel_width;
	unsigned short *pen_16;	
	unsigned int *pen_32;	

	unsigned int red, green, blue;	

	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;

    switch (var.bits_per_pixel)
	{
		case 8:
		{
			*pen_8 = color;
			break;
		}
		case 16:
		{
			/* 565 */
			red   = (color >> 16) & 0xff;
			green = (color >> 8) & 0xff;
			blue  = (color >> 0) & 0xff;
			color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
			*pen_16 = color;
			break;
		}
		case 32:
		{
			*pen_32 = color;
			break;
		}
		default:
		{
			printf("can't surport %dbpp\n", var.bits_per_pixel);
			break;
		}
    }
}

static void lcd_screen_fill(unsigned int color)
{
	int i,j;
	if(color == 0)
	{
		memset(fbmem, 0, screen_size);
	}
	else
	{
		for(i = 0; i < var.yres; i++)
		{
			for(j = 0; j < var.xres; j++)
			{
				lcd_put_pixel(j,i,color);
			}
		}
	}
}

static int lcd_init(void)
{
    fd_fb = open(FB_PATH, O_RDWR);
	if (fd_fb < 0)
	{
		printf("can't open "FB_PATH"\n");
		return -1;
	}
    if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &var))
	{
		printf("can't get var\n");
		return -1;
	}
    line_width  = var.xres * var.bits_per_pixel / 8;
	pixel_width = var.bits_per_pixel / 8;
	screen_size = var.xres * var.yres * var.bits_per_pixel / 8;
	fbmem = (unsigned char *)mmap(NULL , screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);
    if (fbmem == (unsigned char *)-1)
	{
		printf("can't mmap\n");
		return -1;
	}

    /* 清屏: 全部设为黑色 */
	lcd_screen_fill(0);
    return 0;
}

static void lcd_get_var(p_display_var parmas)
{
	parmas->res_x = var.xres;
	parmas->res_y = var.yres;
	parmas->bpp   = var.bits_per_pixel;
}

static void lcd_close(void)
{
	lcd_screen_fill(0);
	munmap(fbmem , screen_size);
	close(fd_fb);
}

display_ops lcd_ops = {
	.name = PLUGIN_NAME,
	.init = lcd_init,
	.get_var = lcd_get_var,
	.put_pixel = lcd_put_pixel,
	.fill = lcd_screen_fill
};

int display_plugin_register_lcd(void)
{
	int ret;
	ret = display_plugin_register(&lcd_ops);
	return ret;
}

void display_plugin_unregister_lcd(void)
{
	lcd_close();
	display_plugin_unregister(&lcd_ops);
}


