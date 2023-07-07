#ifndef ARAVISIMAGE_H
#define ARAVISIMAGE_H
#include <cstdint>
typedef struct _ArvBuffer ArvBuffer;
typedef struct _ArvStream ArvStream;

class AravisImage
{
public:
    AravisImage( ArvStream* a_stream, ArvBuffer* a_buffer, uint16_t a_height, uint16_t a_width, uint16_t a_bpp );
    ~AravisImage();
    bool isValid();
    const unsigned char* getData() const;
    std::size_t getSize() const;
    std::size_t getWidth() const;
    std::size_t getHeight() const;
    std::size_t getBitsPerPixel() const;
    uint64_t getTimestamp() const;
    uint64_t getHostTimestamp() const;

private:
    ArvBuffer* m_buffer;
    ArvStream* m_stream;
    uint16_t m_height;
    uint16_t m_width;
    uint16_t m_bpp;
};

#endif // ARAVISIMAGE_H
