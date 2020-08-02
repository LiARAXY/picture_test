#include "module_font_manager.h"
#include "module_font_plugin_freetype.h"

#define PLUGIN_NAME "FreeType"

static FT_Library    library;
static FT_Face       face;
static FT_GlyphSlot  slot;
static FT_Matrix     matrix;                 /* transformation matrix */
static FT_Vector     pen;                    /* untransformed origin  */

static int borderX, borderY;
static unsigned int font_width, font_height;

static void (*draw_dot)(int,int,unsigned int);

static int FT_init(p_font_var var)
{
    int err;
    if(var->fontsize_width > var->border_x || var->fontsize_width > var->border_y || var->fontsize_height > var->border_x || var->fontsize_height > var->border_y)
    {
        printf("Font size is over the border!\n");
        return -1;
    }
    err = FT_Init_FreeType( &library );
    if (err)
    {
        printf("FT_Init_FreeType is error!\n");
		return err;
    }
    err = FT_New_Face( library, var->filename, 0, &face ); /* create face object */
	if (err)
    {
        printf("FT_New_Face is error!\n");
		return err;
    }
    err = FT_Set_Pixel_Sizes(face, var->fontsize_width, var->fontsize_height);
	if (err)
    {
        printf("FT_Set_Pixel_Sizes is error!\n");
		return err;
    }
	err = FT_Select_Charmap(face, ft_encoding_unicode);
	if (err)
    {
        printf("FT_Select_Charmap is error!\n");
		return err;
    }
	borderX  	= var->border_x;
    borderY  	= var->border_y;
	font_width 	= var->fontsize_height;
	font_height = var->fontsize_width;
	draw_dot 	= var->draw_dot;
	slot = face->glyph;
	return 0;
}

void FT_transform(double angle)
{
	double w;
	w = ( 1.0 * angle / 360 ) * 3.14159 * 2;
	matrix.xx = (FT_Fixed)( cos( w ) * 0x10000L );
	matrix.xy = (FT_Fixed)(-sin( w ) * 0x10000L );
	matrix.yx = (FT_Fixed)( sin( w ) * 0x10000L );
	matrix.yy = (FT_Fixed)( cos( w ) * 0x10000L );
	/* set transformation */
    FT_Set_Transform( face, &matrix, &pen);
}

static int FT_draw_bitmap(FT_Bitmap *bitmap,FT_Int x,FT_Int y,unsigned int color)
{
  	FT_Int  i, j, p, q;
  	FT_Int  x_max = x + bitmap->width;
  	FT_Int  y_max = y + bitmap->rows;
  	for ( i = x, p = 0; i < x_max; i++, p++ )
  	{
  	  	for ( j = y, q = 0; j < y_max; j++, q++ )
  	  	{

  	  	  	if ( i < 0 || j < 0 || i >= borderX || j >= borderY )
			{
				printf("error : Font out of range!\n");
				return -1;
			} 
  	  	  	if(bitmap->buffer[q * bitmap->width + p] != 0) draw_dot(i , j , ((color * bitmap->buffer[q * bitmap->width + p]) / 0xff ) );
  	  	}
  	}
	return 0;
}

static void FT_set_pos(FT_Vector *vector,unsigned int x,unsigned int y)
{
	/* 确定座标:
	 * lcd_x = x
	 * lcd_y = y
	 * 笛卡尔座标系:
	 * x = lcd_x = x
	 * y = borderY - lcd_y  
	 * 单位是1/64像素
	 */
	vector->x = x * 64 ;
	vector->y = (borderY - y) * 64;
}

static int FT_putChar(int x,int y,unsigned int color,wchar_t charCode)
{
	int m,n,err;
	err = FT_Load_Char(face,charCode,FT_LOAD_RENDER);
	if(err)
	{
		printf("FT_Load_Char is error!\n");
		return err;
	}
	FT_set_pos(&pen,x, y);
	m = x + slot->bitmap_left;
	n = y + (face->size->metrics.ascender/64) - slot->bitmap_top;
	err = FT_draw_bitmap(&slot->bitmap, m, n, color);
	if(err)
	{
		printf("FT_draw_bitmap is error!\n");
		return err;
	}
	return 0;
}

static int FT_putString(int x,int y,unsigned int color,wchar_t *str)
{
	unsigned int len;
	int i,m,n,err;
	len = wcslen(str);
	m = x;
	n = y;
	for(i = 0; i < len ; i++)
	{
		if (x < 0 || y < 0 || x > borderX || y > (borderY - font_height))
		{
			printf("Position is out of the range of screen!\n");
			return -1;
		}
		if(m > (borderX - font_width))
		{
			m = 0;
			n = n + font_height;
		}
		if(str[i] == '\n' )
		{
			m = 0;
			n = n + font_height;
		}
		else
		{
			err = FT_putChar(m,n,color,str[i]);
			if(err)
			{
				printf("FT_putChar is error!\n");
				return err;
			}
			m = m + (int)(slot->metrics.horiAdvance / 64);
		}
	}
	return 0;
}

font_ops freetype_ops = {
	.name = PLUGIN_NAME,
	.init = FT_init,
	.putChar = FT_putChar,
	.putString = FT_putString
};

int font_plugin_register_freetype(void)
{	
	int err;
	err = font_plugin_register(&freetype_ops);
	return err;
}

void font_plugin_unregister_freetype(void)
{	
	font_plugin_unregister(&freetype_ops);
}
