#include "module_picture_manager.h"
#include "module_picture_plugins.h"

static p_picture_linklist_node picture_linklist_head,picture_linklist_tail;
static p_picture_format_ops format_selected;

static void (*put_pixel)(int,int,unsigned int);
static PF_zoom_function zoom_func;

static unsigned border_x,border_y;

/*  ↓ 链表相关操作 */
static int picture_list_creat(void)
{   
    picture_linklist_head = malloc(sizeof(picture_linklist_node));
    if(picture_linklist_head == NULL)
    {
        printf("picture_linklist malloc failed!\n");
        return -1;
    }
    picture_linklist_head->operation = NULL;
    picture_linklist_head->prev = NULL;
    picture_linklist_head->next = NULL;
    picture_linklist_tail = picture_linklist_head;
    return 0;
}

static p_picture_linklist_node picture_node_serch(char *name)
{
    p_picture_linklist_node tmp;
    tmp = picture_linklist_head;
    while(1)
    {
        if(!strcmp(tmp->operation->name,name)) return tmp;
        if(tmp != picture_linklist_tail) tmp = tmp->next;
        else break;
    }
    return NULL;
}

static void picture_node_modify(p_picture_linklist_node node,p_picture_format_ops data)
{
    node->operation = data;
}

static int picture_node_add(p_picture_format_ops data)
{
    if((picture_linklist_tail == picture_linklist_head) && (picture_linklist_head->operation == NULL))
    {
        picture_node_modify(picture_linklist_head, data);
    }
    else
    {
        p_picture_linklist_node tmp;
        tmp = malloc(sizeof(picture_linklist_node));
        if(tmp == NULL)
        {
            printf("picture_linklist node add malloc failed!\n");
            return -1;
        }
        picture_linklist_tail->next = tmp;
        tmp->prev = picture_linklist_tail;
        picture_linklist_tail = tmp;
        picture_node_modify(tmp,data);
    }
    return 0;
}

static int picture_node_delete(p_picture_linklist_node node)
{
    if (node == NULL) return -1;
    p_picture_linklist_node tmp;
    if(node->prev != NULL)
    {
        tmp = node->prev;
        tmp->next = node->next;
        if(node->next == NULL) picture_linklist_tail = tmp;
    }
    if(node->next != NULL)
    {
        tmp = node->next;
        tmp->prev = node->prev;
        if(node->prev == NULL) picture_linklist_head = tmp;
    }
    free(node);
    node = NULL;
    return 0;
}

static void picture_list_destory(void)
{
    picture_linklist_head = NULL;
    picture_linklist_tail = NULL;
}

/* ↓ plugins注册 */
static int picture_plugins_register(void)
{
    int err;
    err = picture_plugin_register_format_bmp();
    if(err)
    {
        printf("ERROR : bmp plugin register failed\n");
        return err;
    }
    err = picture_plugin_register_format_jpeg();
    if(err)
    {
        printf("ERROR : jpeg plugin register failed\n");
        return err;
    }
    err = picture_plugin_register_format_png();
    if(err)
    {
        printf("ERROR : png plugin register failed\n");
        return err;
    }
    return 0;
}

static void picture_plugins_unregister(void)
{
    picture_plugin_unregister_format_bmp();
    picture_plugin_unregister_format_jpeg();
    picture_plugin_unregister_format_png();
}


static int picture_zoom_mode_set(unsigned int zoom_mode)
{
    int ret;
    ret = zoom_mode_set(zoom_mode,&zoom_func);
    if(ret)
    {
        printf("ERROR : zoom mode set failed!\n");
    }
    return 0;
}
/*  ↓ 提供给上层的接口函数 */
int picture_plugin_format_select(char *format_name)
{
    p_picture_linklist_node tmp;
    tmp = picture_node_serch(format_name);
    if(tmp == NULL ) 
    {
        printf("picture_plugin_select failed!\n");
        return -1;
    }
    else
    {
        format_selected = tmp->operation;
        printf("\npicture_plugin_select = %d\n",(unsigned int)format_selected);
        return 0;
    }
}

void picture_plugin_deselect(void)
{
    format_selected = NULL;
}

int picture_plugin_init(void *format_var)
{
    int ret;
    ret = format_selected->init(format_var);
    return ret;
}

int picture_module_init(p_picture_var var)
{
    picture_linklist_head = NULL;
    picture_linklist_tail = NULL;
    put_pixel = NULL;
    zoom_func = NULL;
    int ret;
    ret = picture_list_creat();
    if(ret) return ret;
    ret = picture_plugins_register();
    if(ret) return ret;
    ret = picture_zoom_mode_set(var->zoom_mode);
    if(ret) return ret;
    put_pixel = var->draw_dot;
    border_x = var->border_x;
    border_y = var->border_y;
    return 0;
}

void picture_module_remove(void)
{
    if(format_selected != NULL) 
    {
        picture_plugin_deselect();
    }
    picture_plugins_unregister();
    picture_list_destory();
}

int picture_formatCorrect(void *format_var)
{
    int ret;
    ret = format_selected->formatCorrect(format_var);
    return ret;
}

int picture_getInfo(void *format_var, p_picture_info info)
{
    int ret;
    ret = format_selected->getInfo(format_var,info);
    return ret;
}

int picture_decode(void *format_var, p_picture_info info,unsigned int *data)
{
    int ret;
    ret = format_selected->getRGBdata(format_var,info,data);
    return ret;
}

int picture_open(void *format_var, char *filename, char *open_mode)
{
    int ret;
    ret =  format_selected->open(format_var, filename,open_mode);
    return ret;
}

int picture_close(void *format_var)
{
    int ret;
    ret = format_selected->close(format_var);
    return ret;
}

void picture_plugin_release(void *format_var)
{
    format_selected->release(format_var);
}

int picture_zoom(p_picture_info info_src,p_picture_info info_dst, unsigned int *data_src, unsigned int *data_dst)
{
    int ret;
    ret = zoom_func(info_src,info_dst,data_src,data_dst);
    return ret;
}

int picture_display(unsigned int x, unsigned int y, unsigned int *rgb_data, p_picture_info info)
{
    int i;
    unsigned int m,n;
    if (x > border_x)
    {
        printf("ERROR : x is out of range!");
        return -1;
    }
    if(y > border_y)
    {
        printf("ERROR : y is out of range!");
        return -1;
    }
    m = x;
    n = y;
    for(i = 0; i < info->data_len; i++)
    {
        put_pixel(m,n,rgb_data[i]);
        m++;
        if( (m - x > (info->resX - 1)) || (m > border_x) )
        {
            m = x;
            n++;
        }
        if ( (n - y > (info->resY - 1)) || (n > border_y) ) break;
    }
    return 0;
}

/*  ↓ 底层注册函数 */
int picture_plugin_register(p_picture_format_ops data)
{
    int err;
    err = picture_node_add(data);
    return err;
}

void picture_plugin_unregister(p_picture_format_ops data)
{
    p_picture_linklist_node tmp;
    tmp = picture_node_serch(data->name);
    if(tmp != NULL) picture_node_delete(tmp);
}

