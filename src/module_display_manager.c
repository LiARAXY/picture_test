#include "module_display_manager.h"
#include "module_display_plugins.h"

static p_display_linklist_node display_linklist_head,display_linklist_tail;
static p_display_ops plugin_selected;

/*  ↓ 链表相关操作 */
static int display_list_creat(void)
{  
    display_linklist_head = malloc(sizeof(display_linklist_node));
    if(display_linklist_head == NULL)
    {
        printf("display_linklist malloc failed!\n");
        return -1;
    }
    display_linklist_head->operation = NULL;
    display_linklist_head->prev = NULL;
    display_linklist_head->next = NULL;
    display_linklist_tail = display_linklist_head;
    return 0;
}

static p_display_linklist_node display_node_serch(char *name)
{   
    p_display_linklist_node tmp;
    tmp = display_linklist_head;
    while(1)
    {
        if(!strcmp(tmp->operation->name,name)) return tmp;
        if(tmp != display_linklist_tail) tmp = tmp->next;
        else return NULL;
    }
}

static void display_node_modify(p_display_linklist_node node,p_display_ops data)
{
    node->operation = data;
}

static int display_node_add(p_display_ops data)
{
    if((display_linklist_tail == display_linklist_head) && (display_linklist_head->operation == NULL))
    {
        display_node_modify(display_linklist_head, data);
    }
    else
    {
        p_display_linklist_node tmp;
        tmp = malloc(sizeof(display_linklist_node));
        if(tmp == NULL)
        {
            printf("display_linklist node add malloc failed!\n");
            return -1;
        }
        display_linklist_tail->next = tmp;
        tmp->prev = display_linklist_tail;
        display_linklist_tail = tmp;
        display_node_modify(tmp,data);
    }
    return 0;
}

static int display_node_delete(p_display_linklist_node node)
{
    if (node == NULL) return -1;
    p_display_linklist_node tmp;
    if(node->prev != NULL)
    {
        tmp = node->prev;
        tmp->next = node->next;
        if(node->next == NULL) display_linklist_tail = tmp;
    }
    if(node->next != NULL)
    {
        tmp = node->next;
        tmp->prev = node->prev;
        if(node->prev == NULL) display_linklist_head = tmp;
    }
    free(node);
    node = NULL;
    return 0;
}

static void display_list_destory(void)
{
    display_linklist_head = NULL;
    display_linklist_tail = NULL;
}

/* 注册plugin */

static int display_plugins_register(void)
{
    int err;
    err = display_plugin_register_lcd();
    if(err) 
    {
        printf("LCD register failed\n");
        return err;
    }
    return 0;
}

static void display_plugins_unregister(void)
{
    display_plugin_unregister_lcd();
}

/*  ↓ 提供给上层的接口函数 */
int display_plugin_select(char *plugin_name)
{
    p_display_linklist_node tmp;
    tmp = display_node_serch(plugin_name);
    if(tmp == NULL) return -1;
    else 
    {
        plugin_selected = tmp->operation;
        return 0;
    }
}

int display_plugin_init(void)
{
    int ret;
    ret = plugin_selected->init();
    return ret;
}

void display_plugin_deselect(void)
{
    plugin_selected = NULL;
}

int display_module_init(void)
{
    display_linklist_head = NULL;
    display_linklist_tail = NULL;
    plugin_selected = NULL;
    int ret;
    ret = display_list_creat();
    if(ret) return ret;
    ret = display_plugins_register();
    if(ret) return ret;
    return 0;
}

void display_module_remove(void)
{
    if(plugin_selected != NULL) display_plugin_deselect();
    display_plugins_unregister();
    display_list_destory();
}

void display_get_var(p_display_var parmas)
{
    plugin_selected->get_var(parmas);
}

void display_put_pixel(int x,int y,unsigned int color)
{
    plugin_selected->put_pixel(x,y,color);
}

void display_fill(unsigned int val)
{
    plugin_selected->fill(val);
}

/*  ↓ 底层注册 */
int display_plugin_register(p_display_ops data)
{
    int err;
    err = display_node_add(data);
    return err;
}

void display_plugin_unregister(p_display_ops data)
{
    p_display_linklist_node tmp;
    tmp = display_node_serch(data->name);
    if(tmp != NULL) display_node_delete(tmp);
}
