#ifndef ARAVISCAMERA_H
#define ARAVISCAMERA_H

#include <string>
#include <vector>
#include <cstddef>
#include <memory>
#include <aravisstream.h>

typedef struct _ArvCamera ArvCamera;

class AravisCamera
{
public:
    AravisCamera( const std::string& a_device );
    ~AravisCamera();

    void setBool( const std::string& a_feature, bool a_value );
    bool getBool( const std::string& a_feature );

    void setFloat( const std::string& a_feature, double  a_value );
    double getFloat( const std::string& a_feature );

    void setEnum( const std::string& a_feature, const std::string& a_value );
    std::string getEnum( const std::string& a_feature );

    void setInt( const std::string& a_feature, int64_t a_value );
    int64_t getInt( const std::string& a_feature );

    void executeCommand( const std::string& a_feature );

    void startAcquisition();
    void stopAcquisition();

    std::unique_ptr< AravisStream > getStream();

    static std::vector< std::string > getList();

private:
    ArvCamera* m_camera{ nullptr };
};

#endif // ARAVISCAMERA_H
