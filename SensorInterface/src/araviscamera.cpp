#include <string>

#include <araviscamera.h>
#include <arv.h>

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

AravisCamera::AravisCamera( const std::string& a_device )
{
    GError* error = nullptr;

    m_camera = arv_camera_new( a_device.c_str(), &error );

    if( ARV_IS_CAMERA( m_camera ) )
    {
        return;
    }
    else
    {
        g_object_unref( m_camera );
        m_camera = nullptr;
        throw_if_error( error );
    }

    throw_if_error( error );
}

AravisCamera::~AravisCamera()
{
    if( m_camera )
    {
        g_object_unref( m_camera );
    }
}

void AravisCamera::setBool( const std::string& a_feature, const bool a_value )
{
    GError* error = nullptr;
    arv_camera_set_boolean( m_camera, a_feature.c_str(), a_value, &error );
    throw_if_error( error );
}

bool AravisCamera::getBool( const std::string& a_feature )
{
    GError* error = nullptr;
    gboolean retval = arv_camera_get_boolean( m_camera, a_feature.c_str(), &error );
    throw_if_error( error );

    return retval;
}

void AravisCamera::setFloat( const std::string& a_feature, double a_value )
{
    GError* error = nullptr;
    arv_camera_set_float( m_camera, a_feature.c_str(), a_value, &error );
    throw_if_error( error );
}

double AravisCamera::getFloat( const std::string& a_feature )
{
    GError* error = nullptr;
    double retval = arv_camera_get_float( m_camera, a_feature.c_str(), &error );
    throw_if_error( error );

    return retval;
}

void AravisCamera::setEnum( const std::string& a_feature, const std::string& a_value )
{
    GError* error = nullptr;
    arv_camera_set_string( m_camera, a_feature.c_str(), a_value.c_str(), &error );
    throw_if_error( error );
}

std::string AravisCamera::getEnum( const std::string& a_feature )
{
    GError* error = nullptr;
    const char* value = arv_camera_get_string( m_camera, a_feature.c_str(), &error );
    throw_if_error( error );

    std::string str;
    str.append( value );

    g_free( &value );

    return str;
}

void AravisCamera::setInt( const std::string& a_feature, int64_t a_value )
{
    GError* error = nullptr;
    arv_camera_set_integer( m_camera, a_feature.c_str(), a_value, &error );
    throw_if_error( error );
}

int64_t AravisCamera::getInt( const std::string& a_feature )
{
    GError* error = nullptr;
    int64_t retval = arv_camera_get_integer( m_camera, a_feature.c_str(), &error );
    throw_if_error( error );

    return retval;
}

void AravisCamera::executeCommand( const std::string& a_feature )
{
    GError* error = nullptr;
    arv_camera_execute_command( m_camera, a_feature.c_str(), &error );
    throw_if_error( error );
}

void AravisCamera::startAcquisition()
{
    GError* error = nullptr;
    arv_camera_start_acquisition( m_camera, &error );
    throw_if_error( error );
}

void AravisCamera::stopAcquisition()
{
    GError* error = nullptr;
    arv_camera_stop_acquisition( m_camera, &error );
    throw_if_error( error );
}

std::unique_ptr< AravisStream > AravisCamera::getStream()
{
    int64_t width = getInt( "Width" );
    int64_t height = getInt( "Height" );
    return std::make_unique< AravisStream >( m_camera, width, height, 8, 128 );
}

std::vector< std::string > AravisCamera::getList()
{
    arv_update_device_list();

    unsigned int devices = arv_get_n_devices();

    std::vector< std::string > list;

    for( std::size_t id = 0; id < devices; id++ )
    {
        list.push_back( arv_get_device_id( id ) );
    }
    return list;
}
