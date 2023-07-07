#ifndef ARAVISSTREAM_H
#define ARAVISSTREAM_H
#include <cstdint>
#include <memory>

class AravisImage;
typedef struct _ArvStream ArvStream;
typedef struct _ArvCamera ArvCamera;

class AravisStream
{
public:
    AravisStream( ArvCamera* a_camera, uint16_t a_height, uint16_t a_width, uint16_t a_bpp, uint16_t a_numberOfBuffers );
    ~AravisStream();
    std::unique_ptr< AravisImage > getImage();

private:
    ArvStream* m_stream{ nullptr };
};

#endif // ARAVISSTREAM_H
