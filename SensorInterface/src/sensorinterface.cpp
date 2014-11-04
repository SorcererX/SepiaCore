#include "sensorinterface.h"

std::vector<SensorInterface* > SensorInterface::sm_instances;

SensorInterface::SensorInterface()
{
    sm_instances.push_back( this );
}

void SensorInterface::terminateAll()
{
    for( unsigned int i = 0; i <  sm_instances.size(); i++ )
    {
        sm_instances[i]->stop();
    }
}

void SensorInterface::joinAll()
{
    for( unsigned int i = 0; i < sm_instances.size(); i++ )
    {
        sm_instances[i]->join();
    }
}
