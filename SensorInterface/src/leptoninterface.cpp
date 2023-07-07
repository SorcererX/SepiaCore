#include "leptoninterface.h"
#include <sys/ioctl.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <linux/types.h>
#include <unistd.h>
#include <cstdint>

namespace LeptonInterface
{

const int FrameWidth = 80;
const int FrameHeight = 60;
const int RowPacketWords = FrameWidth + 2;
const int RowPacketBytes = 2*RowPacketWords;
const int FrameWords = FrameWidth*FrameHeight;


bool open_device( int& a_fd, const std::string &a_device )
{
    a_fd = open( a_device.c_str(), O_RDWR);
    if (a_fd < 0)
    {
        return false;
    }
    return true;
}

bool init_spi( int& a_fd, unsigned char& a_mode, unsigned char& a_bits, unsigned int& a_speed )
{
    if( set_spi_mode( a_fd, a_mode )
     && get_spi_mode( a_fd, a_mode )
     && set_bits_per_word( a_fd, a_bits )
     && get_bits_per_word( a_fd, a_bits )
     && set_max_speed( a_fd, a_speed )
     && get_max_speed( a_fd, a_speed ) )
    {
        return true;
    }
    return false;
}

void init_buffer( int& a_fd,
                  struct spi_ioc_transfer& a_buffer,
                  unsigned int& a_speed,
                  unsigned char& a_bits  )
{
    a_buffer.tx_buf = (unsigned long) malloc( RowPacketBytes );
    a_buffer.len = RowPacketBytes;
    a_buffer.delay_usecs = 0; // no delay.
    a_buffer.speed_hz = a_speed;
    a_buffer.bits_per_word = a_bits;

}

int set_spi_mode( int& a_fd, unsigned char& a_mode )
{
    return ioctl( a_fd, SPI_IOC_WR_MODE, &a_mode );
}

int get_spi_mode( int& a_fd, unsigned char& a_mode )
{
    return ioctl( a_fd, SPI_IOC_WR_MODE, &a_mode );
}

int set_bits_per_word( int& a_fd, unsigned char& a_bits )
{
    return ioctl( a_fd, SPI_IOC_WR_BITS_PER_WORD, &a_bits );
}

int get_bits_per_word( int& a_fd, unsigned char& a_bits )
{
    return ioctl( a_fd, SPI_IOC_RD_BITS_PER_WORD, &a_bits );
}

int set_max_speed( int& a_fd, unsigned int& a_speed )
{
    return ioctl( a_fd, SPI_IOC_WR_MAX_SPEED_HZ, &a_speed );
}

int get_max_speed( int& a_fd, unsigned int& a_speed )
{
    return ioctl( a_fd, SPI_IOC_RD_MAX_SPEED_HZ, &a_speed );
}

int get_packet( int& a_fd, struct spi_ioc_transfer& a_buffer, unsigned char* a_packetData )
{
    a_buffer.rx_buf = reinterpret_cast< unsigned long >( a_packetData );
    return ioctl( a_fd, SPI_IOC_MESSAGE(1), &a_buffer );
}

bool get_raw_frame( int& a_fd, struct spi_ioc_transfer& a_buffer, unsigned char* a_frame )
{
    int resets = 0; // Number of times we've reset the 0...59 loop for packets
    int errors = 0; // Number of error-packets received

    int y;

    for( y = 0; y < FrameHeight; y++ )
    {
        unsigned char* packet = reinterpret_cast< unsigned char* >( a_frame[ y * RowPacketBytes ] );

        if( get_packet( a_fd, a_buffer, packet ) < 1 )
        {
            return false;
        }
        int packetNumber;

        if( (packet[ 0 ] & 0xf) == 0xf )
        {
            packetNumber = -1;
        }
        else
        {
            packetNumber = packet[ 1 ];
        }

        if( packetNumber == -1 )
        {
            usleep( 1000 );
            if( ++errors > 300 )
            {
                break;
            }
            continue;
        }
        if( packetNumber != y )
        {
            usleep( 1000 );
            break;
        }
    }

    if( y < FrameHeight )
    {
        if( ++resets >= 750 )
        {
            // Packet reset counter hit 750
            resets = 0;
            usleep( 750000 );
        }
        return false;
    }
    return true;
}

void decode_frame( unsigned char* a_input, unsigned short* a_output )
{
    uint16_t minValue = 65535;
    uint16_t maxValue = 0;
    for ( int y = 0; y < FrameHeight; ++y ) {
        a_input += 4;
        for (int x = 0; x < FrameWidth; ++x ) {
            unsigned short value = a_input[0];
            value <<= 8;
            value |= a_input[1];
            a_input += 2;
            if (value > maxValue) maxValue = value;
            if (value < minValue) minValue = value;
            *(a_output++) = value;
        }
    }
}

}
