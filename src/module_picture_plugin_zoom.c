#include "module_picture_manager.h"
#include "module_picture_plugin_zoom.h"

/* 最邻近插值法 */
static int zoom_mode_0(p_picture_info info_src,p_picture_info info_dst, unsigned int *data_src, unsigned int *data_dst)
{
    if(data_src == NULL)
    {
        printf("ERROR : zoom data_src is invalid!\n");
        return -1;
    }
    if(data_dst == NULL)
    {
        printf("ERROR : zoom data_src is invalid!\n");
        return -1;
    }
    double m,n;
    int i,j,x_src,y_src,x_dst,y_dst,x_tmp,y_tmp;    
    x_src = info_src->resX;
    y_src = info_src->resY;
    x_dst = info_dst->resX;
    y_dst = info_dst->resY;
    m = (double)x_dst/x_src;
    n = (double)y_dst/y_src;
    for (j = 0; j < y_dst; j++)
    {
        for (i = 0; i < x_dst; i++)
        {
            x_tmp = round(i/m);
            y_tmp = round(j/n);
            data_dst[i + (j*x_dst)] = data_src[x_tmp + (y_tmp*x_src)];
        }
    }
    return 0;
}

/* 双线性插值法 */
static int zoom_mode_1(p_picture_info info_src,p_picture_info info_dst, unsigned int *data_src, unsigned int *data_dst)
{
    if(data_src == NULL)
    {
        printf("ERROR : zoom data_src is invalid!\n");
        return -1;
    }
    if(data_dst == NULL)
    {
        printf("ERROR : zoom data_src is invalid!\n");
        return -1;
    }
    double m,n,x_tmp,y_tmp,r,g,b;
    unsigned int xRes_src,yRes_src,xRes_dst,yRes_dst,cnt;
    int i,j,x1,x2,y1,y2;
    unsigned int f1r,f2r,f1g,f2g,f1b,f2b,tmpVal;
    xRes_src = info_src->resX;
    yRes_src = info_src->resY;
    xRes_dst = info_dst->resX;
    yRes_dst = info_dst->resY;
    m = (double)xRes_dst/xRes_src;
    n = (double)yRes_dst/yRes_src;
    cnt = 0;
    for (i = 0; i < yRes_src; i++)
    {
        y_tmp = floor(i*n);
        for (j = 0; j < xRes_dst; j++)
        {
            x_tmp = j / m;
            x1 = floor(x_tmp);
            x2 = ceil(x_tmp);
            if(x1 == x2) tmpVal = data_src[i + x1*xRes_src];
            else
            {
                f1r = (data_src[i + x1*xRes_src]>>(8*2)) & 0xff;
                f1g = (data_src[i + x1*xRes_src]>>(8*1)) & 0xff;
                f1b = (data_src[i + x1*xRes_src]>>(8*0)) & 0xff;
                f2r = (data_src[i + x2*xRes_src]>>(8*2)) & 0xff;
                f2g = (data_src[i + x2*xRes_src]>>(8*1)) & 0xff;
                f2b = (data_src[i + x2*xRes_src]>>(8*0)) & 0xff;
                r = (f1r*(x_tmp - x1) + f2r*(x2 - x_tmp))/(x2 -x1);
                g = (f1g*(x_tmp - x1) + f2g*(x2 - x_tmp))/(x2 -x1);
                b = (f1b*(x_tmp - x1) + f2b*(x2 - x_tmp))/(x2 -x1);
                tmpVal = ((unsigned int)r<<(8*2)) + ((unsigned int)g<<(8*1)) + (unsigned int)b;
            }
            data_dst[j + (int)y_tmp*xRes_dst] = tmpVal;
            cnt ++;
        }
    }
    for (i = 0; i < xRes_src; i++)
    {
        x_tmp = floor(i*n);
        for (j = 0; j < yRes_dst; j++)
        {
            y_tmp = j / n;
            y1 = floor(y_tmp);
            y2 = ceil(y_tmp);
            if(y1 == y2) tmpVal = data_src[y1 + i*yRes_src];
            else
            {
                f1r = (data_src[y1 + i*yRes_src]>>(8*2)) & 0xff;
                f1g = (data_src[y1 + i*yRes_src]>>(8*1)) & 0xff;
                f1b = (data_src[y1 + i*yRes_src]>>(8*0)) & 0xff;
                f2r = (data_src[y2 + i*yRes_src]>>(8*2)) & 0xff;
                f2g = (data_src[y2 + i*yRes_src]>>(8*1)) & 0xff;
                f2b = (data_src[y2 + i*yRes_src]>>(8*0)) & 0xff;
                r = (f1r*(y_tmp - y1) + f2r*(y2 - y_tmp))/(y2 - y1);
                g = (f1g*(y_tmp - y1) + f2g*(y2 - y_tmp))/(y2 - y1);
                b = (f1b*(y_tmp - y1) + f2b*(y2 - y_tmp))/(y2 - y1);
                tmpVal = ((unsigned int)r<<(8*2)) + ((unsigned int)g<<(8*1)) + (unsigned int)b;
            }
            data_dst[j + (int)x_tmp*yRes_dst] = tmpVal;
            cnt++;
        }
    }
    printf("cnt = %d,data len = %d\n",cnt,info_dst->data_len);
    return 0;
}

int zoom_mode_set(unsigned int mode,PF_zoom_function *zoom)
{
    switch (mode)
    {
        case 0: *zoom = zoom_mode_0; break;
        case 1: *zoom = zoom_mode_1; break;
        default:
        {
            printf("ERROR : don't surpport this zoom mode!\n");
            return -1;
            break;
        }
    }
    return 0;
}