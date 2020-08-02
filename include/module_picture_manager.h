#ifndef __MODULE_PICTUREFORMAT_MANAGER_H__
#define __MODULE_PICTUREFORMAT_MANAGER_H__

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct __picture_info
{
    unsigned int resX;
    unsigned int resY;
    unsigned int data_len;
    unsigned int bpp;
}picture_info,*p_picture_info;

typedef struct __picture_format_ops
{
    char    *name;
    int     (*init)(char*);
    int     (*formatCorrect)(void*);
    int     (*getInfo)(void*, p_picture_info);
    int     (*getRGBdata)(void*,unsigned int*);
    void    (*release)(void*);
    int     (*open)(void*,char*,char*);
    int     (*close)(void*);
}picture_format_ops,*p_picture_format_ops;

typedef struct __picture_var
{
    void (*draw_dot)(int,int,unsigned int);
    unsigned int zoom_mode;
    unsigned int border_x;
    unsigned int border_y;
}picture_var,*p_picture_var;


typedef struct __picture_linklist_node
{
    picture_format_ops  operation;
    struct __picture_linklist_node *prev;
    struct __picture_linklist_node *next;
}picture_linklist_node,*p_picture_linklist_node;

int picture_plugin_format_select(char *format_name);
int picture_module_init(p_picture_var var);
void picture_plugin_deselect(void);
void picture_module_remove(void);

int picture_plugin_init(void *format_var, char *filename);
int picture_formatCorrect(void *format_var);
int picture_getInfo(void *format_var, p_picture_info info);
int picture_decode(void *format_var, unsigned int *data);
int picture_zoom(p_picture_info info_src,p_picture_info info_dst, unsigned int *data_src, unsigned int *data_dst);
void picture_plugin_release(void *format_var);
int picture_open(void *format_var, char *filename,char *open_mode);
int picture_close(void *format_var);
int picture_display(unsigned int x, unsigned int y, char *rgb_data, p_picture_info info);

int picture_plugin_register(picture_format_ops data);
void picture_plugin_unregister(picture_format_ops data);

#endif
