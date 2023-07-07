#include <aravisimage.h>
#include <arv.h>
#include <iostream>

AravisImage::AravisImage( ArvStream* a_stream, ArvBuffer* a_buffer, uint16_t a_height, uint16_t a_width, uint16_t a_bpp ) : m_buffer( a_buffer )
                                                                                                                          , m_stream( a_stream )
                                                                                                                          , m_height( a_height )
                                                                                                                          , m_width(  a_width  )
                                                                                                                          , m_bpp( a_bpp )
{
}

AravisImage::~AravisImage()
{
    if( m_buffer )
    {
        arv_stream_push_buffer( m_stream, m_buffer );
    }
}

bool AravisImage::isValid()
{
    return ( m_buffer != nullptr );
}

const unsigned char* AravisImage::getData() const
{
    return reinterpret_cast< const unsigned char* >( arv_buffer_get_data( m_buffer, nullptr ) );
}

std::size_t AravisImage::getSize() const
{
    return getHeight() * getWidth() * getBitsPerPixel() / 8;
}

std::size_t AravisImage::getWidth() const
{
    return m_width;
}

std::size_t AravisImage::getHeight() const
{
    return m_height;
}

std::size_t AravisImage::getBitsPerPixel() const
{
    return m_bpp;
}

uint64_t AravisImage::getTimestamp() const
{
    return arv_buffer_get_timestamp( m_buffer );
}

uint64_t AravisImage::getHostTimestamp() const
{
    return arv_buffer_get_system_timestamp( m_buffer );
}
