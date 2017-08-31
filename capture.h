#ifndef _CAPTURE_H_
#include  "types.h"
#define _CAPTUREJPG_H_
#define  IMAGEWIDTH    640
#define  IMAGEHEIGHT   480
#define CLEAR(x) memset (&(x), 0, sizeof (x))
typedef enum {  

    IO_METHOD_READ, IO_METHOD_MMAP, IO_METHOD_USERPTR,  

} io_method;  

struct buffer {  

    void * start;  

    size_t length;

}; 
struct frame{
	float left_eye;
	float right_eye;
	float mouth_eye;
	bool is_sleepy;
};
void init_(void);
static void open_device(void);
static void close_device(void);
static void init_device(void);
static void stop_capturing(void);
static void start_capturing(void);
static void uninit_device(void);
#endif

