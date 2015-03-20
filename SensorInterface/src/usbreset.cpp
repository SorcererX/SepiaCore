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
