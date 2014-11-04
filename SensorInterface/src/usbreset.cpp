#include "usbreset.h"
#include <libusb-1.0/libusb.h>
#include <unistd.h>


UsbReset::UsbReset()
{
}

void UsbReset::addId( uint64_t idVendor, uint64_t idProduct )
{
    m_ids.insert( std::make_pair( idVendor, idProduct ) );
}

bool UsbReset::shouldBeReset( uint64_t idVendor, uint64_t idProduct )
{
    std::pair< uint64_t, uint64_t > checkPair( idVendor, idProduct );

    DeviceList::iterator it = m_ids.find( checkPair );
    if( it != m_ids.end() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void UsbReset::reset()
{
    libusb_context *ctx;

    if( libusb_init( &ctx ) )
    {
        return;
    }

    libusb_device **list;
    libusb_device_handle *handle;
    libusb_device_descriptor desc;

    ssize_t cnt = libusb_get_device_list( ctx, &list );

    for( ssize_t i = 0; i < cnt; i++ )
    {
        if( libusb_get_device_descriptor( list[i], &desc ) != 0 )
        {
            continue;
        }

        if( shouldBeReset( desc.idVendor, desc.idProduct ) )
        {
            if( libusb_open( list[i], &handle ) == 0 )
            {
                libusb_reset_device( handle );
                libusb_close( handle );
            }
        }

    }

    if( cnt >= 0 )
    {
        libusb_free_device_list( list, 1 );
    }
    libusb_exit( ctx );
}
