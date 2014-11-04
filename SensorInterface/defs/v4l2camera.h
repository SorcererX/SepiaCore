#ifndef V4L2CAMERA_H
#define V4L2CAMERA_H
#include <string>
#include <linux/videodev2.h>
#include <cstdint>
#include <map>
#include <vector>
#include <boost/thread.hpp>

namespace V4L2Interface
{
    struct buffer;
}

class V4L2Camera
{
    public:
        V4L2Camera( std::string filename );
        ~V4L2Camera();
        void listFormats();
        std::vector< v4l2_fmtdesc >& getFormats();
        std::vector< v4l2_frmsizeenum >& getFrameSizes( u_int32_t format );
        std::vector< v4l2_frmivalenum >& getFrameIntervals( u_int32_t format, u_int32_t width, u_int32_t height );
        void setFormat( u_int32_t format, u_int32_t width, u_int32_t height );
        void init_mmap();
        void open();
        void readyCapture();
        void startCapture();
        void stopCapture();
        bool readyFrame();
        bool readFrame( unsigned char* input, unsigned int &size, struct timeval& timestamp );
        struct v4l2_format getCurrentFormat();
        void printFormatList();
        int getFileDesc();
        bool setAbsoluteExposure( int msec );
        bool getAbsoluteExposure( int& msec );
        bool setPowerLineFrequencyTo50HZ();
        bool setManualExposure();
        bool setAutoExposure();
        bool setGain( int value );
        bool getGain( int& value );

    protected:
        void open_device();

    private:
        int m_fd;
        std::string m_deviceName;
        struct v4l2_capability m_capability;
        struct v4l2_pix_format m_format;
        struct v4l2_format m_completeFormat; // WTF?
        struct v4l2_cropcap m_cropCapability;
        struct v4l2_crop m_crop;
        //struct v4lconvert_data* m_convert;
        std::vector< v4l2_fmtdesc > m_formatList;
        std::map< u_int32_t, std::vector< v4l2_frmsizeenum > > m_frameSizeList;
        std::map< u_int32_t, std::map< std::pair< u_int32_t, u_int32_t >, std::vector< v4l2_frmivalenum > > > m_fpsList;
        unsigned int m_nBuffers;

        struct V4L2Interface::buffer* buffers;
};


#endif // V4L2CAMERA_H
