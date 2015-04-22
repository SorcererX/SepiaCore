#ifndef V4L2INTERFACE_H
#define V4L2INTERFACE_H
#include <string>
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <vector>

namespace V4L2Interface
{

struct buffer {
        void*  start;
        size_t length;
};

void stream_on( int fd );
void stream_off( int fd );
bool wait_for_data( int fd, int timeout );
bool dequeue_buffer( int fd, struct v4l2_buffer& buf );
bool queue_buffer( int fd, int index );
bool query_buffer( int fd, struct v4l2_buffer& buf, int index );
void open_device( int& fd, std::string deviceName );
void query_capabilities( int fd, std::string deviceName );
void reset_cropcap( int fd );
bool set_ctrl( int fd, int id, int value );
bool get_ctrl( int fd, int id, int& value );
bool set_format( int fd, u_int32_t pixelformat, u_int32_t width, u_int32_t height );
bool get_format( int fd, struct v4l2_format &a_format );
bool request_buffers( int fd, int& count );
void memory_map( int fd, struct v4l2_buffer& buf, buffer& buffer );
void enum_framesizes( int fd, u_int32_t format, std::vector< v4l2_frmsizeenum >& list );
void enum_frameintervals( int fd, u_int32_t format, u_int32_t width, u_int32_t height, std::vector< v4l2_frmivalenum >& list );
void enum_formats( int fd, std::vector< v4l2_fmtdesc >& list );
}

#endif // V4L2INTERFACE_H
