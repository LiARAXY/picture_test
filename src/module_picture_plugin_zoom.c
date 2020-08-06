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
    double m,n,x_tmp,y_tmp,fr1,fr2;
    unsigned int i,j,x_src,y_src,x_dst,y_dst,x1,x2,y1,y2;
    double f11,f12,f21,f22;
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
            x1 = floor(x_tmp);
            x2 = ceil(x_tmp);
            y1 = floor(y_tmp);
            y2 = ceil(y_tmp);
            f11 = data_src[ y_src*x1 + y1];
            f12 = data_src[ y_src*x1 + y2];
            f21 = data_src[ y_src*x2 + y1];
            f22 = data_src[ y_src*x2 + y2];
            fr1 = f11*(x2*m - j)/((x2 - x1)*m) + f21*(j - x1*m)/((x2 - x1)*m);
            fr2 = f12*(x2*m - j)/((x2 - x1)*m) + f22*(j - x1*m)/((x2 - x1)*m);
            data_dst[j + i*x_dst] = round( ((double)(y2*n - i)/((y2 -y1)*n))*fr1 + ((double)(i - y1*n)/((y2 -y1)*n))*fr2 );
        }
    }
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