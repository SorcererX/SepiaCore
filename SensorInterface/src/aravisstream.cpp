#include <string>
#include <aravisstream.h>
#include <arv.h>
#include <aravisimage.h>

namespace
{
    void throw_if_error( GError* a_error )
    {
        if( a_error )
        {
            std::string str = a_error->message;
            g_clear_error( &a_error );
            throw str;
        }
    }
}

AravisStream::AravisStream( ArvCamera* a_camera, uint16_t a_height, uint16_t a_width, uint16_t a_bpp, uint16_t a_numberOfBuffers )
{
    GError* error = nullptr;
    m_stream = arv_camera_create_stream( a_camera, nullptr, nullptr, &error );
    throw_if_error( error );

    for( std::size_t i = 0; i < a_numberOfBuffers; i++ )
    {
        // ownership of buffer is handled by m_stream. The buffers will be free'd when m_stream is destroyed.
        arv_stream_push_buffer( m_stream, arv_buffer_new( ( a_height * a_width * a_bpp ) / 8, nullptr ) );
    }
}

AravisStream::~AravisStream()
{
    if( ARV_IS_STREAM( m_stream ) )
    {
        g_object_unref( m_stream );
        m_stream = nullptr;
    }
}

std::unique_ptr< AravisImage > AravisStream::getImage()
{
    return std::make_unique< AravisImage >( m_stream, arv_stream_pop_buffer( m_stream ) );
}
