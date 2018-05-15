#ifndef FB_H_
#define FB_H_


int init_FrameBuffer(void)  ;
int exit_Framebuffer(void);  
int write_data_to_fb(void *fbp, int fbfd, void *img_buf, unsigned int img_width, unsigned int img_height, unsigned int img_bits);


//帧缓冲中的rgb结构
typedef struct {
 unsigned char b; // 蓝色分量
 unsigned char g; // 绿色分量
 unsigned char r; // 红色分量
 unsigned char rgbReserved; // 保留字节（用作Alpha通道或忽略）
} rgb32_frame;


//rgb结构
typedef struct {
 unsigned char r; // 红色分量
 unsigned char g; // 绿色分量
 unsigned char b; // 蓝色分量
 unsigned char rgbReserved; // 保留字节（用作Alpha通道或忽略）
} rgb32;



#endif



