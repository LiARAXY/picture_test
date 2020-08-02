#include "module_font_manager.h"
#include "module_font_plugins.h"

static p_font_linklist_node font_linklist_head,font_linklist_tail;
static p_font_ops plugin_selected;

/*  ↓ 链表相关操作 */
static int font_list_creat(void)
{   
    font_linklist_head = NULL;
    font_linklist_tail = NULL;
    font_linklist_head = malloc(sizeof(font_linklist_node));
    if(font_linklist_head == NULL)
    {
        printf("font_linklist malloc failed!\n");
        return -1;
    }
    font_linklist_head->operation = NULL;
    font_linklist_head->prev = NULL;
    font_linklist_head->next = NULL;
    font_linklist_tail = font_linklist_head;
    return 0;
}

static p_font_linklist_node font_node_serch(char *name)
{
    p_font_linklist_node tmp;
    tmp = font_linklist_head;
    while(1)
    {
        if(!strcmp(tmp->operation->name,name)) return tmp;
        if(tmp != font_linklist_tail) tmp = tmp->next;
        else break;
    }
    return NULL;
}

static void font_node_modify(p_font_linklist_node node,p_font_ops data)
{
    node->operation = data;
}

static int font_node_add(p_font_ops data)
{
    p_font_linklist_node tmp;
    if((font_linklist_tail == font_linklist_head) && (font_linklist_head->operation == NULL))
    {
        font_node_modify(font_linklist_head, data);
    }
    else
    {
        tmp = malloc(sizeof(font_linklist_node));
        if(tmp == NULL)
        {
            printf("font_linklist node add malloc failed!\n");
            return -1;
        }
        else
        {
            font_node_modify(tmp, data);
            tmp->prev = font_linklist_tail;
            tmp->next = NULL;
            font_linklist_tail = tmp;
        }
    }
    return 0;
}

static int font_node_delete(p_font_linklist_node node)
{
    if (node == NULL) return -1;
    p_font_linklist_node tmp;
    if(node->prev != NULL)
    {
        tmp = node->prev;
        tmp->next = node->next;
        if(node->next == NULL) font_linklist_tail = tmp;
    }
    if(node->next != NULL)
    {
        tmp = node->next;
        tmp->prev = node->prev;
        if(node->prev == NULL) font_linklist_head = tmp;
    }
    free(node);
    node = NULL;
    return 0;
}

static void font_list_destory(void)
{
    font_linklist_head = NULL;
    font_linklist_tail = NULL;
}

/* ↓ plugins注册 */
static int font_plugins_register(void)
{
    int err;
    err = font_plugin_register_freetype();
    if(err)
    {
        printf("Freetypes register failed\n");
        return err;
    }
    return 0;
}

static void font_plugins_unregister(void)
{
    font_plugin_unregister_freetype();
}

/*  ↓ 提供给上层的接口函数 */
int font_plugin_select(char *plugin_name)
{
    p_font_linklist_node tmp;
    tmp = font_node_serch(plugin_name);
    if(tmp == NULL ) 
    {
        printf("font_plugin_select failed!\n");
        return -1;
    }
    else
    {
        plugin_selected = tmp->operation;
        return 0;    
    }
}

void font_plugin_deselect(void)
{
    plugin_selected = NULL;
}

int font_plugin_init(p_font_var var)
{
    int ret;
    ret = plugin_selected->init(var);
    return ret;
}

int font_module_init(void)
{
    int ret;
    ret = font_list_creat();
    if(ret) return ret;
    ret = font_plugins_register();
    if(ret) return ret;
    return 0;
}

void font_module_remove(void)
{
    font_plugins_unregister();
    font_list_destory();
}

int font_putChar(int x,int y,unsigned int color,wchar_t charCode)
{
    int ret;
    ret = plugin_selected->putChar(x,y,color,charCode);
    return ret;
}

int font_putString(int x,int y,unsigned int color,wchar_t *str)
{   
    int ret;
    ret = plugin_selected->putString(x,y,color,str);
    return ret;
}

/*  ↓ 底层注册函数 */
int font_plugin_register(p_font_ops data)
{
    int err;
    err = font_node_add(data);
    return err;
}

void font_plugin_unregister(p_font_ops data)
{
    p_font_linklist_node tmp;
    tmp = font_node_serch(data->name);
    if(tmp != NULL) font_node_delete(tmp);
}

