#ifndef USBRESET_H
#define USBRESET_H
#include <set>
#include <cstdint>
#include <utility>


class UsbReset
{
public:
    UsbReset();
    void addId( uint64_t idVendor, uint64_t idProduct );
    void reset();

protected:
    bool shouldBeReset( uint64_t idVendor, uint64_t idProduct );

private:
    typedef std::pair< uint64_t, uint64_t > DeviceId;
    typedef std::set< DeviceId > DeviceList;
    DeviceList m_ids;
};

#endif // USBRESET_H
