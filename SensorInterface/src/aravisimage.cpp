#include <aravisimage.h>
#include <arv.h>

AravisImage::AravisImage( ArvStream* a_stream, ArvBuffer* a_buffer ) : m_buffer( a_buffer )
                                                                     , m_stream( a_stream )
{
    if( m_buffer )
    {
        m_data = reinterpret_cast< const unsigned char* >( arv_buffer_get_data( m_buffer, &m_size ) );
    }
    else
    {
        m_data = nullptr;
        m_size = 0;
    }
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
    return m_data;
}

std::size_t AravisImage::getSize() const
{
    return m_size;
}

uint64_t AravisImage::getTimestamp() const
{
    return arv_buffer_get_timestamp( m_buffer );
}

uint64_t AravisImage::getHostTimestamp() const
{
    return arv_buffer_get_system_timestamp( m_buffer );
}
