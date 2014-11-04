#ifndef SENSORINTERFACE_H
#define SENSORINTERFACE_H
#include<vector>

class SensorInterface
{
    public:
        static void terminateAll();
        static void joinAll();
        virtual void stop() = 0;
        virtual void join() = 0;
    protected:
        SensorInterface();
    private:
        static std::vector< SensorInterface* > sm_instances;
};

#endif // SENSORINTERFACE_H
