#ifndef __MODULE_FONT_FREETYPE_H__
#define __MODULE_FONT_FREETYPE_H__

#include <math.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

int font_plugin_register_freetype(void);
void font_plugin_unregister_freetype(void);

#endif