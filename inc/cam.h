#ifndef CAM_H_
#define CAM_H_


typedef struct VideoBuffer {
    void   *start; 	//视频缓冲区的起始地址
    size_t  length;	//缓冲区的长度
    size_t  offset;	//缓冲区的长度
} VideoBuffer;



int open_cameral(char* path);

void close_cameral(void);

void get_camInfo(void);

int set_format();

int get_buf(void);


void map_buf(void);
void startcon();
int get_picture(unsigned char *buffer);

int stopcon(void);  
int bufunmap(void); 

int WaitCamerReady(unsigned int second);


#endif






