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
    unsigned int xRes_src,yRes_src,xRes_dst,yRes_dst;
    int i,j,x1,x2,y1,y2;
    unsigned int f11r,f12r,f21r,f22r,f11g,f12g,f21g,f22g,f11b,f12b,f21b,f22b,tmpVal;
    xRes_src = info_src->resX;
    yRes_src = info_src->resY;
    xRes_dst = info_dst->resX;
    yRes_dst = info_dst->resY;
    m = (double)xRes_dst/xRes_src;
    n = (double)yRes_dst/yRes_src;
    for (i = 0; i < yRes_dst; i++)
    {
        for (j = 0; j < xRes_dst; j++)
        {
            x_tmp = j / m;
            y_tmp = i / n;
            x1 = floor(x_tmp);
            x2 = ceil(x_tmp);
            y1 = floor(y_tmp);
            y2 = ceil(y_tmp);
            if((x1 == x2) && (y1 != y2))
            {
                f11r = (data_src[y1 + x1*yRes_src]>>(8*2)) & 0xff;
                f11g = (data_src[y1 + x1*yRes_src]>>(8*1)) & 0xff;
                f11b = (data_src[y1 + x1*yRes_src]>>(8*0)) & 0xff;
                f12r = (data_src[y2 + x1*yRes_src]>>(8*2)) & 0xff;
                f12g = (data_src[y2 + x1*yRes_src]>>(8*1)) & 0xff;
                f12b = (data_src[y2 + x1*yRes_src]>>(8*0)) & 0xff;
                r = (f11r*(y_tmp - y1) + f12r*(y2 - y_tmp))/(y2 - y1);
                g = (f11g*(y_tmp - y1) + f12g*(y2 - y_tmp))/(y2 - y1);
                b = (f11b*(y_tmp - y1) + f12b*(y2 - y_tmp))/(y2 - y1);
            }
            else if ((y1 == y2) && (x1 != x2))
            {
                f11r = (data_src[y1 + x1*xRes_src]>>(8*2)) & 0xff;
                f11g = (data_src[y1 + x1*xRes_src]>>(8*1)) & 0xff;
                f11b = (data_src[y1 + x1*xRes_src]>>(8*0)) & 0xff;
                f21r = (data_src[y1 + x2*xRes_src]>>(8*2)) & 0xff;
                f21g = (data_src[y1 + x2*xRes_src]>>(8*1)) & 0xff;
                f21b = (data_src[y1 + x2*xRes_src]>>(8*0)) & 0xff;
                r = (f11r*(x_tmp - x1) + f21r*(x2 - x_tmp))/(x2 -x1);
                g = (f11g*(x_tmp - x1) + f21g*(x2 - x_tmp))/(x2 -x1);
                b = (f11b*(x_tmp - x1) + f21b*(x2 - x_tmp))/(x2 -x1);
            }
            else if((x1 == x2) && (y1 == y2)) tmpVal = data_src[x1 + y1*xRes_src];
            else
            {
                f11r = (data_src[y1 + x1*xRes_src]>>(8*2)) & 0xff;
                f11g = (data_src[y1 + x1*xRes_src]>>(8*1)) & 0xff;
                f11b = (data_src[y1 + x1*xRes_src]>>(8*0)) & 0xff;
                f21r = (data_src[y1 + x2*xRes_src]>>(8*2)) & 0xff;
                f21g = (data_src[y1 + x2*xRes_src]>>(8*1)) & 0xff;
                f21b = (data_src[y1 + x2*xRes_src]>>(8*0)) & 0xff;
                f12r = (data_src[y2 + x1*yRes_src]>>(8*2)) & 0xff;
                f12g = (data_src[y2 + x1*yRes_src]>>(8*1)) & 0xff;
                f12b = (data_src[y2 + x1*yRes_src]>>(8*0)) & 0xff;
                f22r = (data_src[y2 + x2*yRes_src]>>(8*2)) & 0xff;
                f22g = (data_src[y2 + x2*yRes_src]>>(8*1)) & 0xff;
                f22b = (data_src[y2 + x2*yRes_src]>>(8*0)) & 0xff;
                r = (f11r*(x_tmp - x1)*(y_tmp - y1) + f12r*(x_tmp - x1)*(y2 - y_tmp) + f21r*(x2 - x_tmp)*(y_tmp - y1) + f22r*(x2 - x_tmp)*(y2 - y_tmp))/((x2 - x1)*(y2 - y1));
                g = (f11g*(x_tmp - x1)*(y_tmp - y1) + f12g*(x_tmp - x1)*(y2 - y_tmp) + f21g*(x2 - x_tmp)*(y_tmp - y1) + f22g*(x2 - x_tmp)*(y2 - y_tmp))/((x2 - x1)*(y2 - y1));
                b = (f11b*(x_tmp - x1)*(y_tmp - y1) + f12b*(x_tmp - x1)*(y2 - y_tmp) + f21b*(x2 - x_tmp)*(y_tmp - y1) + f22b*(x2 - x_tmp)*(y2 - y_tmp))/((x2 - x1)*(y2 - y1));
            }
            tmpVal = ((unsigned int)r<<(8*2)) + ((unsigned int)g<<(8*1)) + (unsigned int)b;
            data_dst[i + j*xRes_dst] = tmpVal;
            cnt ++;
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