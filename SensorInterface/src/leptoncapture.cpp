#include "leptoncapture.h"
#include "leptoninterface.h"

#include <sepia/writer.h>
#include <functional>

LeptonCapture::LeptonCapture( const std::string& a_device ) : m_bits( 8 ),
                                                              m_speed( 16000000 ),
                                                              m_terminate( false )
{
    LeptonInterface::open_device( m_fd, a_device );

    unsigned char mode = 0;
    LeptonInterface::init_spi( m_fd, mode, m_bits, m_speed );
    LeptonInterface::init_buffer( m_fd, m_buffer, m_speed, m_bits );
}

LeptonCapture::~LeptonCapture()
{

}

void LeptonCapture::start()
{
    m_thread = new std::thread( std::bind( &LeptonCapture::own_thread, this ) );
}

void LeptonCapture::join()
{
    m_thread->join();
}

void LeptonCapture::stop()
{
    m_terminate = true;
}

bool LeptonCapture::isTerminated()
{
    return m_terminate;
}

void LeptonCapture::own_thread()
{
    m_writer = new sepia::Writer( "FLIR_LEPTON", 1, 80, 60, 16 );

    std::vector< unsigned short > tempbuf;
    tempbuf.assign( 80*60, 0 );

    while( !m_terminate )
    {
        LeptonInterface::get_raw_frame( m_fd, m_buffer, reinterpret_cast< unsigned char*>( tempbuf.data() ) );
        LeptonInterface::decode_frame( reinterpret_cast< unsigned char* >( tempbuf.data() ),
                                       reinterpret_cast< unsigned short* >( m_writer->getAddress( 0 ) ) );
        m_writer->update();
    }
}
