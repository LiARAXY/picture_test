#ifndef __DISPLAY_PLUGIN_LCD_H__
#define __DISPLAY_PLUGIN_LCD_H__

#include <sys/mman.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/select.h>

int display_plugin_register_lcd(void);
void display_plugin_unregister_lcd(void);

#endif
