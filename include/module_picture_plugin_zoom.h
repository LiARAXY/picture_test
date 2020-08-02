#ifndef __MODULE_PICTUREFORMAT_ZOOM_H__
#define __MODULE_PICTUREFORMAT_ZOOM_H__

int zoom_mode_set(unsigned int mode,int (*zoom)(p_picture_info, p_picture_info, int*, int*));

#endif