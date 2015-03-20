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

#include "ximeacontroller.h"
#include <sepia/comm/dispatcher.h>
#include <string>
#include <cstring>

XimeaController::XimeaController()
{
}

void XimeaController::addCameraId( unsigned int a_id )
{
   m_cameras.insert( a_id );
}

void XimeaController::removeCameraId( unsigned int a_id )
{
   std::unordered_set<unsigned int>::iterator it = m_cameras.find( a_id );
   m_cameras.erase( it );
}

void XimeaController::open()
{
   sendControl( cuttlefish_msgs::XimeaControl_CommandType_OPEN );
}

void XimeaController::close()
{
   sendControl( cuttlefish_msgs::XimeaControl_CommandType_CLOSE );
}

void XimeaController::start()
{
   sendControl( cuttlefish_msgs::XimeaControl_CommandType_START );
}

void XimeaController::stop()
{
   sendControl( cuttlefish_msgs::XimeaControl_CommandType_STOP );
}

void XimeaController::terminate()
{
   sendControl( cuttlefish_msgs::XimeaControl_CommandType_TERMINATE );
}

void XimeaController::startTrigger()
{
    sendControl( cuttlefish_msgs::XimeaControl_CommandType_TRIGGER_START );
}

void XimeaController::stopTrigger()
{
    sendControl( cuttlefish_msgs::XimeaControl_CommandType_TRIGGER_STOP );
}

void XimeaController::setParameter( const std::string a_parameter, const int a_value )
{
   m_set.clear_float_value();
   m_set.set_int_value( a_value );
   m_set.set_parameter( a_parameter );
   for( unsigned int id : m_cameras )
   {
      m_set.set_camera_no( id );
      sepia::comm::Dispatcher< cuttlefish_msgs::XimeaSet >::send( &m_set );
   }
}

void XimeaController::setParameter( const std::string a_parameter, const float a_value )
{
   m_set.clear_int_value();
   m_set.set_float_value( a_value );
   m_set.set_parameter( a_parameter );
   for( unsigned int id : m_cameras )
   {
      m_set.set_camera_no( id );
      sepia::comm::Dispatcher< cuttlefish_msgs::XimeaSet >::send( &m_set );
   }
}

void XimeaController::sendControl( const cuttlefish_msgs::XimeaControl_CommandType a_command )
{
   m_control.set_command( a_command );

   for( unsigned int id : m_cameras )
   {
      m_control.set_camera_no( id );
      sepia::comm::Dispatcher< cuttlefish_msgs::XimeaControl >::send( &m_control );
   }
}
