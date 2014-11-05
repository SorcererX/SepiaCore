#include <sepia/reader.h>
#include <sepia/writer.h>
#include <sepia/compression/compress.h>
#include <sepia/compression/enc3b11b.h>

int main()
{
    sepia::Reader reader( "XI_IMG" );
    reader.update();

    sepia::Writer writer( "COMPRESSED", reader.getGroupHeader()->count,
                          reader.getHeader( 0 )->width,
                          reader.getHeader( 0 )->height,
                          reader.getHeader( 0 )->bpp  );
    writer.update();


    sepia::compression::Enc3b11b enc;

    while( true )
    {
        reader.update();
        enc.encode( reinterpret_cast< unsigned char* >( writer.getAddress( 0 ) ),
                    reinterpret_cast< unsigned char* >( reader.getAddress( 0 ) ),
                    reader.getSize( 0 ) );
        writer.update();
    }
}
