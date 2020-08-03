#ifndef __MODULE_PICTURE_ZOOM_H__
#define __MODULE_PICTURE_ZOOM_H__

#include <math.h>

typedef int (*PF_zoom_function)(p_picture_info, p_picture_info,unsigned int*,unsigned int*);
int zoom_mode_set(unsigned int mode,PF_zoom_function zoom);

#endif