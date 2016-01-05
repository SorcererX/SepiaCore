#ifndef LEPTONINTERFACE_H
#define LEPTONINTERFACE_H
#include <string>
#include <linux/spi/spidev.h>

namespace LeptonInterface
{
bool open_device( int& a_fd, const std::string& a_device );
bool init_spi(int& a_fd, unsigned char& a_mode, unsigned char& a_bits, unsigned int& a_speed );
void init_buffer( int& a_fd,
                  struct spi_ioc_transfer& a_buffer,
                  unsigned int& a_speed,
                  unsigned char& a_bits  );
int set_spi_mode( int& a_fd, unsigned char& a_mode );
int get_spi_mode( int& a_fd, unsigned char& a_mode );
int set_bits_per_word( int& a_fd, unsigned char& a_bits );
int get_bits_per_word( int& a_fd, unsigned char& a_bits );
int set_max_speed( int& a_fd, unsigned int& a_speed );
int get_max_speed( int& a_fd, unsigned int& a_speed );
int get_packet( int& a_fd, struct spi_ioc_transfer& a_buffer, unsigned char* a_packetData );
bool get_raw_frame( int& a_fd, struct spi_ioc_transfer& a_buffer, unsigned char* a_frame );
void decode_frame( unsigned char* a_input, unsigned short* a_output );

}

#endif // LEPTONINTERFACE_H
