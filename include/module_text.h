#ifndef __MODULE_TEXT_H__
#define __MODULE_TEXT_H__

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

int utf16_text_process(char *src,wchar_t *charCode,unsigned int mode,unsigned int len);

#endif
