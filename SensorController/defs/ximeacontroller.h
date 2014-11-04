#ifndef XIMEACONTROLLER_H
#define XIMEACONTROLLER_H
#include <string>
#include <sepia/comm/header.pb.h>
#include "messages.pb.h"
#include <unordered_set>

class XimeaController
{
   public:
      XimeaController();
      void addCameraId( unsigned int a_id );
      void removeCameraId( unsigned int a_id );
      void open();
      void close();
      void start();
      void stop();
      void terminate();
      void startTrigger();
      void stopTrigger();
      void setParameter( const std::string a_parameter, const int a_value );
      void setParameter( const std::string a_parameter, const float a_value );
   protected:
      void sendControl( const cuttlefish_msgs::XimeaControl_CommandType a_command );

   private:
      sepia::comm::Header m_header;
      cuttlefish_msgs::XimeaControl m_control;
      cuttlefish_msgs::XimeaSet m_set;
      std::unordered_set< unsigned int > m_cameras;
};

#endif // XIMEACONTROLLER_H
