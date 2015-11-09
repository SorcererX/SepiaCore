#ifndef IMAGEPLAYER_H
#define IMAGEPLAYER_H
#include <fstream>
#include <string>
#include <sepia/util/threadobject.h>

namespace sepia
{
    class Writer;
}


class ImagePlayer : public sepia::util::ThreadObject
{
public:
    ImagePlayer( const std::string& a_outputGroupName );
    ~ImagePlayer();
    void setInputFile( const std::string& a_inputFile );
    void enableLoop( bool a_enable );
    void start();

protected:
    void own_thread();
    void stream( double a_waitBetweenFrames );

private:
    ImagePlayer() = delete;
    std::string m_outputGroupName;
    sepia::Writer* m_writer;
    std::string m_inputFile;
    bool m_loop;
};

#endif // IMAGEPLAYER_H
