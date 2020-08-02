#ifndef __MODULE_DISPLAY_MANAGER_H__
#define __MODULE_DISPLAY_MANAGER_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <stdlib.h>
#include <wchar.h> 

typedef struct __display_var
{
    unsigned int res_x;
    unsigned int res_y;
    unsigned int bpp;
}display_var,*p_display_var;


typedef struct __display_ops
{
    char    *name;
    int     (*init)(void);                     
    void    (*get_var)(p_display_var);
    void    (*put_pixel)(int,int,unsigned int); 
    void    (*fill)(unsigned int);
}display_ops,*p_display_ops;


typedef struct __display_device_node
{
    p_display_ops operation;
    struct __display_device_node * prev;
    struct __display_device_node * next;
}display_linklist_node,*p_display_linklist_node;



int display_plugin_select(char *plugin_name);
int display_plugin_init(void);
int display_module_init(void);
void display_plugin_deselect(void);
void display_module_remove(void);

void display_put_pixel(int x,int y,unsigned int color);
void display_get_var(p_display_var parmas);
void display_fill(unsigned int val);

int display_plugin_register(p_display_ops data);
void display_plugin_unregister(p_display_ops data);

#endif
