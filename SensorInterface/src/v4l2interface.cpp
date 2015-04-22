/*
Copyright (c) 2012-2015, Kai Hugo Hustoft Endresen <kai.endresen@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "v4l2interface.h"

namespace V4L2Interface
{

int xioctl(int fd, int IOCTL_X, void *arg)
{
        int ret = 0;
        int tries= 10;
        do
        {
                ret = v4l2_ioctl(fd, IOCTL_X, arg);
        }
        while (ret && tries-- &&
                        ((errno == EINTR) || (errno == EAGAIN) || (errno == ETIMEDOUT)));

        if (ret && (tries <= 0)) fprintf( stderr, "ioctl (%i) retried %i times - giving up: %s)\n", IOCTL_X, 10, strerror(errno) );

        return (ret);
}

void stream_on( int fd )
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if( -1 == xioctl ( fd, VIDIOC_STREAMON, &type ) )
    {
        std::cerr << "VIDIOC_STREAMON" << std::endl;
        exit( EXIT_FAILURE );
    }
}

void stream_off( int fd )
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if( -1 == xioctl ( fd, VIDIOC_STREAMOFF, &type ) )
    {
        std::cerr << "VIDIOC_STREAMOFF" << std::endl;
        exit( EXIT_FAILURE );
    }
}

bool wait_for_data( int fd, int timeout )
{
    fd_set fds;
    struct timeval tv;

    FD_ZERO (&fds);
    FD_SET ( fd, &fds);

    /* Select Timeout */
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    int r = select ( fd + 1, &fds, NULL, NULL, &tv);

    if (-1 == r)
    {
        if (EINTR == errno)
            return true;

        return false;
    }

    if (0 == r)
    {
        // SELECT TIMEOUT
        return false;
    }
    return true;
}

bool dequeue_buffer( int fd, struct v4l2_buffer& buf )
{
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl ( fd, VIDIOC_DQBUF, &buf ) )
    {
        switch (errno)
        {
            case EAGAIN:
                return 0;

            case EIO://EIO ignored

            default:
                return false;
        }
    }
    return true;
}

bool queue_buffer( int fd, int index )
{
    struct v4l2_buffer buf;
    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = index;
    if ( -1 == xioctl ( fd, VIDIOC_QBUF, &buf) )
    {
        return false;
    }
    return true;
}

bool query_buffer( int fd, struct v4l2_buffer& buf, int index )
{
    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = index;

    if ( -1 == xioctl ( fd, VIDIOC_QUERYBUF, &buf ) )
    {
        std::cerr << "VIDIOC_QUERYBUF" << std::endl;
        exit( EXIT_FAILURE );
    }
    return true;
}


void open_device( int& fd, std::string deviceName )
{

    struct stat st;

    if ( -1 == stat ( deviceName.c_str(), &st) ) {
            fprintf (stderr, "Cannot identify '%s': %d, %s\n",
                     deviceName.c_str(), errno, strerror (errno));
            exit(EXIT_FAILURE);
    }

    if ( !S_ISCHR (st.st_mode) ) {
            fprintf (stderr, "%s is no device\n", deviceName.c_str() );
            exit(EXIT_FAILURE);
    }

    fd = v4l2_open( deviceName.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0 );

    if (-1 == fd) {
            fprintf (stderr, "Cannot open '%s': %d, %s\n",
                     deviceName.c_str(), errno, strerror (errno));
            exit(EXIT_FAILURE);
    }
}

void query_capabilities( int fd, std::string deviceName )
{
    struct v4l2_capability capability;
    // check if V4L2 device
    if (-1 == ioctl ( fd, VIDIOC_QUERYCAP, &capability ) ) {
            if (EINVAL == errno) {
                    fprintf (stderr, "%s is no V4L2 device\n",
                             deviceName.c_str());
                    exit(EXIT_FAILURE);
            } else {
                    exit(EXIT_FAILURE);
            }
    }

    // check if device is a VIDEO_CAPTURE device and supports streaming.
    if( ( capability.capabilities & V4L2_CAP_VIDEO_CAPTURE )
     && ( capability.capabilities & V4L2_CAP_STREAMING ) )
    {
        std::cout << deviceName << ": supports Video Capture Interface and Streaming" << std::endl;
    }
    else
    {
        printf( "Capability not recognized: %x\n", capability.capabilities );
        exit(EXIT_FAILURE);
    }
}

void reset_cropcap( int fd )
{
    struct v4l2_cropcap cropCapability;
    struct v4l2_crop crop;

    //select video input, standard(not used) and tuner(not used) here
    cropCapability.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 == xioctl ( fd, VIDIOC_CROPCAP, &cropCapability))
    {
                /* Errors ignored. */
    }

    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropCapability.defrect; /* reset to default */

    if (-1 == xioctl ( fd, VIDIOC_S_CROP, &crop))
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
    }
}

bool set_ctrl( int fd, int id, int value )
{
    v4l2_control ctrl;
    ctrl.id=id;
    ctrl.value=value;
    if ( -1 == xioctl( fd, VIDIOC_S_CTRL, &ctrl ) )
    {
        return false;
    }
    return true;
}

bool get_ctrl( int fd, int id, int& value )
{
    v4l2_control ctrl;
    ctrl.id=id;
    if ( -1 == xioctl( fd, VIDIOC_G_CTRL, &ctrl ) )
    {
        return false;
    }
    value=ctrl.value;
    return true;
}

bool set_format( int fd, u_int32_t pixelformat, u_int32_t width, u_int32_t height )
{
    struct v4l2_format format;
    format.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width       = width;
    format.fmt.pix.height      = height;
    format.fmt.pix.pixelformat = pixelformat;
    format.fmt.pix.field       = V4L2_FIELD_INTERLACED;

    if ( 0 != xioctl ( fd, VIDIOC_S_FMT, &format) )
    {
       std::cout << "FORMAT NOT SUPPORTED." << std::endl;
       exit( EXIT_FAILURE );
    }
    return true;
}

bool get_format( int fd, struct v4l2_format &a_format )
{
    a_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if ( -1 == xioctl( fd, VIDIOC_G_FMT, &a_format ) )
    {
        return false;
    }
    return true;
}


bool request_buffers( int fd, int& count )
{
    struct v4l2_requestbuffers req;
    req.count               = count;
    req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory              = V4L2_MEMORY_MMAP;

    if ( -1 == xioctl ( fd, VIDIOC_REQBUFS, &req ) ) {
            if ( EINVAL == errno ) {
                std::cerr << "Memory mapping not supported" << std::endl;
                exit (EXIT_FAILURE);
            } else {
                std::cerr << "VIDIOC_REQBUFS" << std::endl;
            }
    }
    count = req.count;

    if (count < 2) {
            std::cerr << "Insufficient buffer memory" << std::endl;
            exit (EXIT_FAILURE);
    }
    return true;
}

void memory_map( int fd, struct v4l2_buffer& buf, buffer& buffer )
{
    buffer.length = buf.length;
    buffer.start = v4l2_mmap (NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd, buf.m.offset );

    if (MAP_FAILED == buffer.start)
    {
        std::cerr << "MMAP failed" << std::endl;
        exit( EXIT_FAILURE );
    }
}

void enum_framesizes( int fd, u_int32_t format, std::vector< v4l2_frmsizeenum >& list )
{
    int ret = 0;
    struct v4l2_frmsizeenum fsize;
    memset(&fsize, 0, sizeof(fsize));
    fsize.index = 0;
    fsize.pixel_format = format;

    while ((ret = xioctl( fd, VIDIOC_ENUM_FRAMESIZES, &fsize)) == 0)
    {
        list.push_back( fsize );
        fsize.index++;
    }
}

void enum_frameintervals( int fd, u_int32_t format, u_int32_t width, u_int32_t height, std::vector< v4l2_frmivalenum >& list )
{
    int ret=0;
    struct v4l2_frmivalenum fival;
    memset(&fival, 0, sizeof(fival));
    fival.index = 0;
    fival.pixel_format = format;
    fival.width = width;
    fival.height = height;

    while ((ret = xioctl( fd, VIDIOC_ENUM_FRAMEINTERVALS, &fival)) == 0)
    {
        list.push_back( fival );
        fival.index++;
    }
}

void enum_formats( int fd, std::vector< v4l2_fmtdesc >& list )
{
    int ret = 0;
    struct v4l2_fmtdesc format;
    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.index = 0;
    while( ( ret = xioctl( fd, VIDIOC_ENUM_FMT, &format ) ) == 0 )
    {
        list.push_back( format );
        format.index++;
    }
}

}
