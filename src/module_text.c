#include "module_text.h"

int utf16_text_process(char *src,wchar_t *charCode,unsigned int mode,unsigned int len)
{
	unsigned int len,i,j,mode;
	for (i = 0,j = 0; i < len; i=i+2)
	{
		if(i<2)
		{
			if ((src[0] == 0xff) && (src[1] == 0xfe))
			{
				mode = 0;
			}
			else if ((src[0] == 0xfe) && (src[1] == 0xff))
			{
				mode = 1;
			}		
		}
		else
		{
			if (mode == 0)
			{
				charCode[j] = (src[i+1] << 8) + src[i];
			}
			else if(mode == 1)
			{
				charCode[j] = (src[i] << 8) + src[i+1];
			}
			j++;
		}
	}
	charCode[j+1]='\0';
	return j;
}