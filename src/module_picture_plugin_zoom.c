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
    for (j = 0; j < y_src; j++)
    {
        for (i = 0; i < x_src; i++)
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
    double m,n,x_tmp,y_tmp;
    unsigned int i,j,x_src,y_src,x_dst,y_dst,x1,x2,y1,y2;
    int f11,f12,f21,f22;
    x_src = info_src->resX;
    y_src = info_src->resY;
    x_dst = info_dst->resX;
    y_dst = info_dst->resY;
    m = (double)x_dst/x_src;
    n = (double)y_dst/y_src;
    for (i = 0; i < y_dst; i++)
    {
        for (j = 0; j < x_dst; j++)
        {
            x_tmp = j / m;
            y_tmp = i / n;
            x1 = (int)(x_tmp - 1);
            x2 = (int)(x_tmp + 1);
            y1 = (int)(y_tmp + 1);
            y2 = (int)(y_tmp - 1);
            f11 = data_src[ y1*x_src + x1 ];
            f12 = data_src[ y2*x_src + x1 ];
            f21 = data_src[ y1*x_src + x2 ];
            f22 = data_src[ y2*x_src + x2 ];
            data_dst[j + i*y_dst] = (int)( ((  f11 * ((double)x2 - x_tmp) * ((double)y2 - y_tmp) )  +
                                            (  f21 * (x_tmp - (double)x1) * ((double)y2 - y_tmp) )  +
                                            (  f12 * ((double)x2 - x_tmp) * (y_tmp - (double)y1) )  +
                                            (  f22 * (x_tmp - (double)x1) * (y_tmp - (double)y1) )) / ((x2 - x1) * (y2 - y1)) );
        }
    }
    return 0;
}

int zoom_mode_set(unsigned int mode,PF_zoom_function zoom)
{
    int ret;
    switch (mode)
    {
        case 0: zoom = zoom_mode_0; break;
        case 1: zoom = zoom_mode_1; break;
        default:
        {
            printf("ERROR : don't surpport this zoom mode!\n");
            ret = -1;
            break;
        }
    }
    return 0;
}