#ifndef __FONT_MANAGER_H__
#define __FONT_MANAGER_H__

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h> 
#include <stdlib.h>

typedef struct __font_var
{
    char *filename;
    unsigned int fontsize_width;
    unsigned int fontsize_height;
    int border_x;
    int border_y;
    void (*draw_dot)(int,int,unsigned int);
}font_var,*p_font_var;


typedef struct __font_ops
{
    char    *name;
    int     (*init)(p_font_var);
    int     (*putChar)(int,int,unsigned int,wchar_t);
    int     (*putString)(int,int,unsigned int,wchar_t*);
}font_ops,*p_font_ops;

typedef struct __font_linklist_node
{
    p_font_ops operation;
    struct __font_linklist_node * prev;
    struct __font_linklist_node * next;
}font_linklist_node,*p_font_linklist_node;

int font_plugin_select(char *plugin_name);
int font_plugin_init(p_font_var var);
int font_module_init(void);
void font_plugin_deselect(void);
void font_module_remove(void);

int font_putChar(int x,int y,unsigned int color,wchar_t charCode);
int font_putString(int x,int y,unsigned int color,wchar_t *str);

int font_plugin_register(p_font_ops data);
void font_plugin_unregister(p_font_ops data);

#endif
