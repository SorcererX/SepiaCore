#ifndef ARAVISIMAGE_H
#define ARAVISIMAGE_H
#include <cstdint>
typedef struct _ArvBuffer ArvBuffer;
typedef struct _ArvStream ArvStream;

class AravisImage
{
public:
    AravisImage( ArvStream* a_stream, ArvBuffer* a_buffer );
    ~AravisImage();
    bool isValid();
    const unsigned char* getData() const;
    std::size_t getSize() const;
    uint64_t getTimestamp() const;
    uint64_t getHostTimestamp() const;

private:
    ArvBuffer* m_buffer;
    ArvStream* m_stream;
    const unsigned char* m_data;
    std::size_t m_size;
};

#endif // ARAVISIMAGE_H
