/*
 * capture.c
 */
#include <stdio.h>  
#include <malloc.h>
#include <stdlib.h>  
#include <string.h>  
#include <assert.h>  
#include <getopt.h>             /* getopt_long() */  
#include <fcntl.h>              /* low-level i/o */  
#include <unistd.h>  
#include <errno.h>  
#include <malloc.h>  
#include <sys/stat.h>  
#include <sys/types.h>  
#include <sys/time.h>  
#include <sys/mman.h>  
#include <sys/ioctl.h>  
#include <asm/types.h>          /* for videodev2.h */  
#include <linux/videodev2.h>
//#include "/usr/include/jpeglib.h"
#include "/home/jeel/work/opencv/opencv-imx6q-depend/include/jpeglib.h"
#include  "capture.h"
#include  "types.h"
#include "opencv2/opencv.hpp"
#include "string.h"
#include "log.h"
#include "savecamera.h"
#include <sys/stat.h>
#include <sys/statfs.h>
extern log_st *log_sleep;
extern bool detect_or_save_flag;
//static int j=0;
using namespace cv;
using namespace std;
extern VideoWriter outputVideo;
extern string savevideo_name;
#ifdef _PC
	static char* dev_name="/dev/video0";
#else
	static char* dev_name="/dev/video2";
#endif

static io_method io =IO_METHOD_MMAP;//IO_METHOD_READ;//IO_METHOD_MMAP;  IO_METHOD_USERPTR
static int fd = -1;  
struct buffer * buffers = NULL;  
static unsigned int n_buffers = 0;  
//static FILE * outf = 0;  
static unsigned int cap_image_size = 0;//to keep the real image size!!  
static u8 cap_flag=0;
char  frame_buffer[IMAGEWIDTH*IMAGEHEIGHT*3]={0};
s8 *photo_path_g;
extern  s8 *direc;
extern Mat img;
extern int camera_error_flag;
//struct v4l2_fmtdesc fmtdesc;
//fmtdesc.index = 0;
//fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

int yuyv_2_rgb888(void)
{
    int           i,j;
    unsigned  char y1,y2,u,v;
    int r1,g1,b1,r2,g2,b2;
    unsigned  char* pointer;
    pointer = (unsigned char*)buffers[0].start;

    for(i=0;i<IMAGEHEIGHT;i++)
    {
        for(j=0;j<IMAGEWIDTH/2;j++)//Ã¿ŽÎÈ¡4žö×ÖœÚ£¬Ò²ŸÍÊÇÁœžöÏñËØµã£¬×ª»»rgb£¬6žö×ÖœÚ£¬»¹ÊÇÁœžöÏñËØµã
        {
            y1 = *( pointer + (i*IMAGEWIDTH/2+j)*4);     
            u = *( pointer + (i*IMAGEWIDTH/2+j)*4 + 1);
            y2 = *( pointer + (i*IMAGEWIDTH/2+j)*4 + 2);
            v  = *( pointer + (i*IMAGEWIDTH/2+j)*4 + 3);

            r1 = y1 + (1.4075 * (v-128));
            g1 = y1 - (0.7169 * (v-128)) - ( 0.3455  * (u-128));
            b1 = y1 + (1.779 * (u-128));

            r2 = y2 + (1.4075 * (v-128));
            g2 = y2 - (0.7169 * (v-128)) - ( 0.3455  * (u-128));
            b2 = y2 + (1.779 * (u-128));


            if(r1>255)
            {
             r1 = 255;
            }
            else if(r1<0){  
             r1 = 0;
            }
            if(b1>255){
             b1 = 255;
             }
            else if(b1<0){
            b1 = 0;   
            } 
            if(g1>255)
            {
            g1 = 255;
            }
            else if(g1<0){
                g1 = 0;    
            }
            if(r2>255)
            {
            r2 = 255; 
            }
            else if(r2<0){
            r2 = 0;
            }
            if(b2>255)
            {
            b2 = 255;
            }
            else if(b2<0){         
            b2 = 0;    
            }
            if(g2>255)
            {      
            g2 = 255;
            }
            else if(g2<0){       
            g2 = 0;        
            }
        *(frame_buffer + (i*IMAGEWIDTH/2+j)*6    ) = (unsigned char)b1;//b1
        *(frame_buffer + (i*IMAGEWIDTH/2+j)*6 + 1) = (unsigned char)g1;
        *(frame_buffer + (i*IMAGEWIDTH/2+j)*6 + 2) = (unsigned char)r1;//r1
        *(frame_buffer + (i*IMAGEWIDTH/2+j)*6 + 3) = (unsigned char)b2;//b2
        *(frame_buffer + (i*IMAGEWIDTH/2+j)*6 + 4) = (unsigned char)g2;
        *(frame_buffer + (i*IMAGEWIDTH/2+j)*6 + 5) = (unsigned char)r2;//r2
      }
    }
    //printf("\nchange to RGB OK \n");
    return TRUE;
}
/*
u8 encode_jpeg(char *lpbuf,int width,int height)
{
    struct jpeg_compress_struct cinfo ;
    struct jpeg_error_mgr jerr ;
    s8* jpegn=NULL;
    JSAMPROW  row_pointer[1] ;
    int row_stride ;
    char *buf=NULL;
    int x ;
    //jpegn=GetMultimediaFilesName(PHOTO);
    //photo_path_g=GetMultimediaFilesPath(direc,jpegn); 
    photo_path_g = "jpeg";
    FILE *fptr_jpg = fopen (photo_path_g,"wb");
    if(fptr_jpg==NULL)
    {
        printf("Encoder:open file failed!/n") ;
        return FALSE;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fptr_jpg);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space =JCS_RGB;//JCS_GRAYSCALE  1;JCS_RGB 

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 100,TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    row_stride = width * 3;
    buf=(char*)malloc(row_stride);
    row_pointer[0] = (unsigned char*)buf;
    while (cinfo.next_scanline < height)
    {
     for (x = 0; x < row_stride; x+=3)
    {

        buf[x]   = lpbuf[x];
        buf[x+1] = lpbuf[x+1];
        buf[x+2] = lpbuf[x+2];

    }
        jpeg_write_scanlines (&cinfo, row_pointer, 1);//critical
        lpbuf += row_stride;
    }

    jpeg_finish_compress(&cinfo);
    fclose(fptr_jpg);
    jpeg_destroy_compress(&cinfo);
    free(buf) ;
    
    return TRUE ;

}  */

static void errno_exit(const char * s)
{  

    fprintf(stderr, "%s error %d, %s\r\n", s, errno, strerror(errno));
    LogDebug(log_sleep, "%s error %d, %s\r\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}  

static int xioctl(int fd, int request, void * arg) 
{  

    int r;  
    do  
    r = ioctl(fd, request, arg);  
    while (-1 == r && EINTR == errno);  
    return r;  
}  


static int read_frame(void)
{  

    struct v4l2_buffer buf;  
    unsigned int i;  
    //unsigned char jpegn[128] = {0};
    //unsigned char yuvn[128] = {0};
   cap_flag++;
   
    switch (io) {  

    case IO_METHOD_READ:  

        if (-1 == read(fd, buffers[0].start, buffers[0].length)) {  

            switch (errno) {  

            case EAGAIN:  

                return 0;  

            case EIO: 

            default:  
                errno_exit("read");  

            }  

        }  

        

        printf("image_size = %d,\t IO_METHOD_READ buffer.length=%d\r",  

                cap_image_size, buffers[0].length);  

                break;  

    case IO_METHOD_MMAP:  

        CLEAR (buf);  

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  

        buf.memory = V4L2_MEMORY_MMAP; 
       

        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) 
        {  

            switch (errno)
           {  

            case EAGAIN:  

                return 0;  

            case EIO:  

                /* Could ignore EIO, see spec. */  

                /* fall through */  

            default:
                errno_exit("VIDIOC_DQBUF");
            }  

        }  
       // printf("buf.index=%d\n",buf.index);
        assert(buf.index < n_buffers);  

                if(cap_flag>TRUE)
                 {
                    cap_flag=FALSE;
                    //usleep(1000*120);
                 //   Mat yuyv;
                 //   Mat img;
                  /*  yuyv.create(480*3/2,640,CV_8U);
                    memcpy(yuyv.data, buffers[buf.index].start, buffers[buf.index].length);
                    cvtColor(yuyv, img, CV_YUV2RGB_YV12);
                   // IplImage *frame_img=IplImage(img);*/
                    yuyv_2_rgb888();
                    img.data=(uchar*)frame_buffer;
                    struct timeval tv;
					struct timezone tz;
					gettimeofday(&tv,&tz);
					DBG("capture time : %d\n",tv.tv_sec);
					int mount_check=system("ls /dev/ | grep mmcblk2p1");
					#ifdef _PC
						mount_check=0;
					#endif
					DBG("save video mount_check %d \r\n",mount_check);
					if(mount_check!=0)
					{
						//system("umount -l /mnt/sleep &");
						system("mount -t ext4 /dev/mmcblk2p1 /mnt/sleep");
					}
					else if (!img.empty() && detect_or_save_flag==true && mount_check==0)
					//if (!img.empty() && detect_or_save_flag==true)
                    {
                    	struct statfs diskInfo;
                    	statfs(save_path, &diskInfo);
                    	unsigned long long blocksize = diskInfo.f_bsize;    //每个block里包含的字节数
                    	unsigned long long availableDisk = diskInfo.f_bavail * blocksize;   //可用空间大小
                    	DBG("availabledisk %llu B  %llu MB %llu GB\r\n",availableDisk,availableDisk>>20,availableDisk>>30);
                    	if(availableDisk>1024*1024*1024)
                    	{
                    		outputVideo << img;
							struct stat statbuf;
							stat(savevideo_name.c_str(),&statbuf);
							uint32_t max_save_video_size=200*1024*1024;
							DBG("max_save_video_size %d\r\n",max_save_video_size);
							DBG("video file size %d\r\n",statbuf.st_size);
							if(statbuf.st_size>max_save_video_size)
							{
								outputVideo.release();
								saveCamera();
							}
                    	}
					}
                   // memcpy(yuyv.data,buffers[buf.index].start,buffers[buf.index].length);
                    //encode_jpeg(frame_buffer,IMAGEWIDTH,IMAGEHEIGHT); //RGB  to Jpeg

                  }       
              
           if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))  

               errno_exit("VIDIOC_QBUF");  
          
               break;  

    case IO_METHOD_USERPTR:  

        CLEAR (buf);  

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  

        buf.memory = V4L2_MEMORY_USERPTR;  

        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) 
         {  

            switch (errno)
           {  

            case EAGAIN:  

                return 0;  

            case EIO:  

                /* Could ignore EIO, see spec. */  

                /* fall through */  

            default:  

                errno_exit("VIDIOC_DQBUF");  

            }  

        }  

        for (i = 0; i < n_buffers; ++i)  

            if (buf.m.userptr == (unsigned long) buffers[i].start && buf.length  

                    == buffers[i].length)  

                break;  

        assert(i < n_buffers);  

        //      printf("length = %d\r", buffers[i].length);  

        //      process_image((void *) buf.m.userptr, buffers[i].length);  

        printf("image_size = %d,\t IO_METHOD_USERPTR buffer.length=%d\r",  

                cap_image_size, buffers[0].length);  

       
       
            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))  

                 errno_exit("VIDIOC_QBUF");  

        break;  

    }
    
    //fclose(outf);  
    
    return 1;  

}  

void mainloop(void) {

    unsigned int count;  

    count = 2;  

    while (count-->0) {

        for (;;) {  

            fd_set fds;  

            struct timeval tv;  

            int r;  

            FD_ZERO(&fds);  

            FD_SET(fd, &fds);  

            /* Timeout. */  

            tv.tv_sec = 2;  

            tv.tv_usec = 0;  

            r = select(fd + 1, &fds, NULL, NULL, &tv);  

            if (-1 == r) {  

                if (EINTR == errno)  

                    continue;  

                errno_exit("select");  

            }  

            if (0 == r) {  

                printf("select timeout\n");  

				stop_capturing();
				uninit_device();
				close_device();
				open_device();
				init_device();
				start_capturing();
				mainloop();
				break;
                //exit(EXIT_FAILURE);

            }  

            if (read_frame())  

                break;  

            /* EAGAIN - continue select loop. */  

        }  

    }  

}  

static void stop_capturing(void) {  

    enum v4l2_buf_type type;  

    switch (io) {  

    case IO_METHOD_READ:  

        /* Nothing to do. */  

        break;  

    case IO_METHOD_MMAP:  

    case IO_METHOD_USERPTR:  

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  

        if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))  

            errno_exit("VIDIOC_STREAMOFF");  

        break;  

    }  

}  

static void start_capturing(void) {  

    unsigned int i;  

    enum v4l2_buf_type type;  

    switch (io) {  

    case IO_METHOD_READ:  

        /* Nothing to do. */  

            break;  

    case IO_METHOD_MMAP:  

        for (i = 0; i < n_buffers; ++i)
        {  

            struct v4l2_buffer buf;  

            CLEAR (buf);  

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  

            buf.memory = V4L2_MEMORY_MMAP;  

            buf.index = i;  

            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))  //·ÅÈë»ºŽæ¶ÓÁÐ

                errno_exit("VIDIOC_QBUF");  

        }  

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  

        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))  

            errno_exit("VIDIOC_STREAMON");  

            break;  
  
    case IO_METHOD_USERPTR:  

        for (i = 0; i < n_buffers; ++i) 
         {  

            struct v4l2_buffer buf;  

            CLEAR (buf);  

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  

            buf.memory = V4L2_MEMORY_USERPTR;  

            buf.index = i;  

            buf.m.userptr = (unsigned long) buffers[i].start;  

            buf.length = buffers[i].length;  

            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))  

                errno_exit("VIDIOC_QBUF");  

        }  

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  

        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))  

            errno_exit("VIDIOC_STREAMON");  

        break;  

    }  

}  

static void uninit_device(void) 
{  

    unsigned int i;  

    switch (io)
    {  

    case IO_METHOD_READ:  

        free(buffers[0].start);  

        break;  

    case IO_METHOD_MMAP:  

        for (i = 0; i < n_buffers; ++i)  

            if (-1 == munmap(buffers[i].start, buffers[i].length))  

                errno_exit("munmap");  

        break;  

    case IO_METHOD_USERPTR:  

        for (i = 0; i < n_buffers; ++i)  

             free(buffers[i].start);  

        break;  

    }  

    free(buffers);  

}  

static void init_read(unsigned int buffer_size)
{  

    buffers = (struct buffer *)calloc(1, sizeof(*buffers));

    if (!buffers) {  

        fprintf(stderr, "Out of memory1\r\n");
        LogDebug(log_sleep, "capture Out of memory1\r\n");
        exit(EXIT_FAILURE);  

    }  

    buffers[0].length = buffer_size;  

    buffers[0].start = malloc(buffer_size);  

    if (!buffers[0].start) {  

        fprintf(stderr, "Out of memory2\r\n");
        LogDebug(log_sleep, "capture Out of memory2\r\n");
        exit(EXIT_FAILURE);  

    }  

}  

static void init_mmap(void) 
{  
    struct v4l2_requestbuffers req;  
    CLEAR (req);  
    req.count = 3;  
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
    req.memory = V4L2_MEMORY_MMAP;  
    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
    {  

        if (EINVAL == errno)
       {  

            fprintf(stderr, "%s does not support "  

                "memory mapping\n", dev_name);  
            LogDebug(log_sleep, "%s does not support " "memory mapping\n", dev_name);
            exit(EXIT_FAILURE);  

        } else {  

            errno_exit("VIDIOC_REQBUFS");  

        }  

    }  

    if (req.count < 2) 
   {  

        fprintf(stderr, "Insufficient buffer memory on %s\n", dev_name);
        LogDebug(log_sleep, "Insufficient buffer memory on %s\n", dev_name);
        exit(EXIT_FAILURE);  

    }  

    buffers = (struct buffer *)calloc(req.count, sizeof(*buffers));

    if (!buffers) 
   {  
        fprintf(stderr, "Out of memory3\r\n");
        LogDebug(log_sleep, "capture Out of memory3\r\n");
        exit(EXIT_FAILURE);  
    }  

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
    {  

        struct v4l2_buffer buf;  
        CLEAR (buf);  
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
        buf.memory = V4L2_MEMORY_MMAP;  
        buf.index = n_buffers;  

        if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))  

            errno_exit("VIDIOC_QUERYBUF");

            buffers[n_buffers].length = buf.length;  

            buffers[n_buffers].start = mmap(NULL /* start anywhere */, buf.length,  

            PROT_READ | PROT_WRITE /* required */,  

            MAP_SHARED /* recommended */, fd, buf.m.offset);  

        if (MAP_FAILED == buffers[n_buffers].start)  

            errno_exit("mmap");  

    }  

}  

static void init_userp(unsigned int buffer_size)
{  

    struct v4l2_requestbuffers req;  
    unsigned int page_size;  
    page_size = getpagesize();  
    buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);  
    CLEAR (req);  

    req.count = 4;  
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
    req.memory = V4L2_MEMORY_USERPTR;  

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {  
        if (EINVAL == errno) {  
            fprintf(stderr, "%s does not support "  
                "user pointer i/o\n", dev_name);  
            LogDebug(log_sleep, "%s does not support "  "user pointer i/o\n", dev_name);
            exit(EXIT_FAILURE);  
        } else {  
            errno_exit("VIDIOC_REQBUFS");
        }  

    }  

    buffers = (struct buffer *)calloc(4, sizeof(*buffers));

    if (!buffers) {  

        fprintf(stderr, "Out of memory4\r\n");
        LogDebug(log_sleep, "capture Out of memory4\r\n");
        exit(EXIT_FAILURE);  
    }  

    for (n_buffers = 0; n_buffers < 4; ++n_buffers) {  

        buffers[n_buffers].length = buffer_size;  

        buffers[n_buffers].start = memalign(/* boundary */page_size,  

                buffer_size);  

        if (!buffers[n_buffers].start) {  

            fprintf(stderr, "Out of memory5\r\n");
            LogDebug(log_sleep, "capture Out of memory5\r\n");
            exit(EXIT_FAILURE);  

        }  

    }  

}  

static void init_device(void) 
{  

    struct v4l2_capability cap;  
    struct v4l2_cropcap cropcap;  
    struct v4l2_crop crop;  
    struct v4l2_format fmt;      
    struct v4l2_fmtdesc fmtdesc;
    v4l2_std_id            std;
    unsigned int min; 
    int ret;

    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) 
    {  

        if (EINVAL == errno)
       {  

            fprintf(stderr, "%s is no V4L2 device\n", dev_name);  
            LogDebug(log_sleep, "%s is no V4L2 device\n", dev_name);
            exit(EXIT_FAILURE);  

        } else {  
            errno_exit("VIDIOC_QUERYCAP");
        }  

    }  

    
 do {		
	ret = ioctl(fd, VIDIOC_QUERYSTD, &std);//Œì²éµ±Ç°ÊÓÆµÉè±žÖ§³ÖµÄ±ê×Œ£¬ÀýÈçPAL»òNTSC	
	if (ret == -1 && errno == EAGAIN)
	{
	    usleep(1);
	}
    } while (ret == -1 && errno ==  EAGAIN);
        fmtdesc.index=0;
         fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	printf("Support format:\n");
	while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc)!=-1)
	{
	   printf("\t%d.%s\n",fmtdesc.index+1,fmtdesc.description);
	   fmtdesc.index++;
	}
  // if(ioctl(fd, VIDIOC_S_STD, &std) == -1)//²éÑ¯»òÑ¡Ôñµ±Ç°ÊÓÆµÊäÈë±ê×Œ
   // {
    //      printf("VIDIOC_S_STD failed on video (%s)\n", strerror(errno));
	//  close(fd);
     //   return ;
  //  }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
    {  
        fprintf(stderr, "%s is no video capture device\n", dev_name);  
        LogDebug(log_sleep, "%s is no video capture device\n", dev_name);
        exit(EXIT_FAILURE);  
    }  

    switch (io)
    {  

    case IO_METHOD_READ:  

        if (!(cap.capabilities & V4L2_CAP_READWRITE)) 
        {  
            fprintf(stderr, "%s does not support read i/o\n", dev_name);
            LogDebug(log_sleep, "%s does not support read i/o\n", dev_name);
            exit(EXIT_FAILURE);  
        }  

        break;  

    case IO_METHOD_MMAP:  

    case IO_METHOD_USERPTR:  

        if (!(cap.capabilities & V4L2_CAP_STREAMING)) 
        {  
            fprintf(stderr, "%s does not support streaming i/o\n", dev_name);  
            LogDebug(log_sleep, "%s does not support streaming i/o\n", dev_name);
            exit(EXIT_FAILURE);  
        }  
        break;  

    }  

    CLEAR (cropcap);  

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  

    if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {  

        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  

#ifndef CROP_BY_JACK  

        crop.c = cropcap.defrect; /* reset to default */  

#else  

        crop.c.left = cropcap.defrect.left;  

        crop.c.top = cropcap.defrect.top;  

        crop.c.width =  720;  

        crop.c.height = 576;    

#endif  

        printf("----->has ability to crop!!\n");  

        printf("cropcap.defrect = (%d, %d, %d, %d)\n", cropcap.defrect.left,  

                cropcap.defrect.top, cropcap.defrect.width,  

                cropcap.defrect.height);  
        //ÉèÖÃÊÓÆµÐÅºÅµÄ±ß¿ò
        if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop))
        {  

            switch (errno) 
            {  

            case EINVAL:  

                /* Cropping not supported. */  

                break;  

            default:  

                /* Errors ignored. */  

                break;  

            }  

            printf("-----!!but crop to (%d, %d, %d, %d) Failed!!\n",  

                    crop.c.left, crop.c.top, crop.c.width, crop.c.height);  

        } else {  

            printf("----->sussess crop to (%d, %d, %d, %d)\n", crop.c.left,  

                    crop.c.top, crop.c.width, crop.c.height);  

        }  

    } else {  

        /* Errors ignored. */  

        printf("!! has no ability to crop!!\n");  

    }  

    printf("-#-#-#-#-#-#-#-#-#-#-#-#-#-\n");  

    CLEAR (fmt);  

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  

    fmt.fmt.pix.width = IMAGEWIDTH ;  

    fmt.fmt.pix.height = IMAGEHEIGHT;  

    fmt.fmt.pix.pixelformat =V4L2_PIX_FMT_YUYV;//;//V4L2_PIX_FMT_YUYV;  //V4L2_PIX_FMT_YUYVV4L2_PIX_FMT_YUYV

    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;  

    {  

        printf("-#-#-#-#-#-#-#-#-#-#-#-#-#-\n");  

        printf("=====will set fmt to (%d, %d)--", fmt.fmt.pix.width,  

                fmt.fmt.pix.height);  

        if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_UYVY)
        {  

            printf("V4L2_PIX_FMT_UYVY\n");  

        } else if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUV420)
        {  
            printf("V4L2_PIX_FMT_YUV420\n");  
        } else if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_NV12)
        {  
            printf("V4L2_PIX_FMT_NV12\n");  
        }  

    }  

    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))  

        errno_exit("VIDIOC_S_FMT");  

    {  

        printf("=====after set fmt\n");  

        printf("    fmt.fmt.pix.width = %d\n", fmt.fmt.pix.width);  

        printf("    fmt.fmt.pix.height = %d\n", fmt.fmt.pix.height);  

        printf("    fmt.fmt.pix.sizeimage = %d\n", fmt.fmt.pix.sizeimage);  

        cap_image_size = fmt.fmt.pix.sizeimage;  

        printf("    fmt.fmt.pix.bytesperline = %d\n", fmt.fmt.pix.bytesperline);  

        printf("-#-#-#-#-#-#-#-#-#-#-#-#-#-\n");  

        printf("\n");  

    }  

    cap_image_size = fmt.fmt.pix.sizeimage;  

    /* Note VIDIOC_S_FMT may change width and height. */  

    printf("-#-#-#-#-#-#-#-#-#-#-#-#-#-\n");  
        if(ioctl(fd, VIDIOC_G_FMT, &fmt) == -1)
        {
            printf("Unable to get format\n");
            return ;
        } 
        {
            //printf("fmt.type:\t\t%d\n",fmt.type);
           // printf("pix.pixelformat:\t%c%c%c%c\n",fmt.fmt.pix.pixelformat & 0xFF, (fmt.fmt.pix.pixelformat >> 8) & 0xFF,(fmt.fmt.pix.pixelformat >> 16) & 0xFF, (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
           // printf("pix.height:\t\t%d\n",fmt.fmt.pix.height);
          //  printf("pix.width:\t\t%d\n",fmt.fmt.pix.width);
          //  printf("pix.field:\t\t%d\n",fmt.fmt.pix.field);
        }

    /* Buggy driver paranoia. */  

    min = fmt.fmt.pix.width * 2;  

    if (fmt.fmt.pix.bytesperline < min)  

        fmt.fmt.pix.bytesperline = min;  

    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;  

    if (fmt.fmt.pix.sizeimage < min)  

        fmt.fmt.pix.sizeimage = min;  

  //  printf("After Buggy driver paranoia\n");  

  //  printf("    >>fmt.fmt.pix.sizeimage = %d\n", fmt.fmt.pix.sizeimage);  

 //   printf("    >>fmt.fmt.pix.bytesperline = %d\n", fmt.fmt.pix.bytesperline);  

 //   printf("-#-#-#-#-#-#-#-#-#-#-#-#-#-\n");  

 //   printf("\n");  

    switch (io) {  

    case IO_METHOD_READ:  

        init_read(fmt.fmt.pix.sizeimage);

        break;  

    case IO_METHOD_MMAP:  

        init_mmap();  

        break;  

    case IO_METHOD_USERPTR:  

        init_userp(fmt.fmt.pix.sizeimage);  

        break;  

    }  

}  

static void close_device(void)
{  

    if (-1 == close(fd))  
        errno_exit("close");  
    fd = -1;  

}  

static void open_device(void)
{  
    struct stat st;  
    if (-1 == stat(dev_name, &st))
   {  
        fprintf(stderr, "Cannot identify '%s': %d, %s\n", dev_name, errno,  
                strerror(errno));
        LogDebug(log_sleep, "Cannot identify '%s': %d, %s\n", dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);  
    }  

    if (!S_ISCHR(st.st_mode)) 
    {  
        fprintf(stderr, "%s is no device\n", dev_name);  
        LogDebug(log_sleep, "%s is no device\n", dev_name);
        exit(EXIT_FAILURE);  
    }  
    fd = open(dev_name, O_RDWR /* required */, 0);  //| O_NONBLOCK
    if (-1 == fd) 
   {  
        fprintf(stderr, "Cannot open '%s': %d, %s\n", dev_name, errno,  
                strerror(errno));  
        LogDebug(log_sleep, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);  
    }  

}  
void init_(void)
{   
   // int reg;
   // int i;  
	//cvNamedWindow("1");
    open_device();  
    init_device();  
    start_capturing();  
    /*mainloop();
    stop_capturing();  
    uninit_device();  
    close_device();  
    return 1;*/
}


