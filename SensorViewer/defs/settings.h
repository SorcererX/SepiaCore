#ifndef SETTINGS_H
#define SETTINGS_H
#include <atomic>

class Settings
{
public:
    enum class show {
        BOTH = 0,
        LEFT = 1,
        RIGHT = 2,
        STEREO = 3
    };
    static std::atomic_int demosaicing_method;
    static std::atomic_bool use_raw;
    static show show_mode;
    static bool force_noscale;
};

#endif // SETTINGS_H
