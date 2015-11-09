#include <imageplayer.h>
#include <sepia/writer.h>
#include <opencv2/opencv.hpp>
#include <unistd.h>

ImagePlayer::ImagePlayer( const std::string& a_outputGroupName ) : m_outputGroupName( a_outputGroupName )
                                                                 , m_loop( false )
                                                                 , m_inputFile( "input.list" )
{
}

ImagePlayer::~ImagePlayer()
{
    // NO OP
}

void ImagePlayer::setInputFile( const std::string& a_inputFile )
{
    m_inputFile = a_inputFile;
}

void ImagePlayer::enableLoop( bool a_enable )
{
    m_loop = a_enable;
}

void ImagePlayer::start()
{
    m_thread = new std::thread( std::bind( &ImagePlayer::own_thread, this ) );
}

void ImagePlayer::own_thread()
{
    stream( 1.0 );
}

void ImagePlayer::stream( double a_waitBetweenFrames )
{
    std::ifstream reclist;

    std::ifstream input;

    reclist.open( m_inputFile );

    if( !reclist.is_open() )
    {
        std::cerr << "ERROR: not able to open " << m_inputFile << "." << std::endl;
        return ;
    }

    for( int i = 0; !m_terminate; i++ )
    {
        if( reclist.eof() )
        {
            std::cout << "EOF" << std::endl;
            if( m_loop )
            {
                reclist.clear();
                reclist.seekg( 0, std::ios::beg );
            }
            else
            {
                reclist.close();
                break;
            }
        }
        std::string filename;
        getline( reclist, filename );

        if( !reclist.good() )
        {
            if( !m_loop )
            {
                break;
            }
            else
            {
                reclist.clear();
                reclist.seekg( 0, std::ios::beg );
                getline( reclist, filename );
            }
        }
        input.open( filename );

        if( !input.is_open() )
        {
            std::cerr << "Failed to open: " << filename << std::endl;
            break;
        }
        std::cout << "Playing " << filename << std::endl;

        cv::Mat image = cv::imread( filename );
        /*
        if( image == NULL )
        {
            std::cerr << "Failed to decode: " << filename << std::endl;
            break;
        }
        */

        if( i == 0 )
        {
            std::cout << "channels: " << image.channels() << std::endl;
            m_writer = new sepia::Writer( m_outputGroupName, 2, image.cols, image.rows, image.channels()*8 );
        }

        m_writer->copyWrite( i % 2 , reinterpret_cast< char* >( image.data ) );

        if( i % 2 == 1 )
        {
            std::cout << "Update()" << std::endl;
            m_writer->update();
            usleep( 1000000 * a_waitBetweenFrames );
        }
    }
    input.close();
}
