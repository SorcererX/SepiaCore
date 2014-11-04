#include <settings.h>
#include <opencv2/opencv.hpp>

std::atomic_int Settings::demosaicing_method(cv::COLOR_BayerBG2BGR_EA);
std::atomic_bool Settings::use_raw(false);
Settings::show Settings::show_mode = Settings::show::BOTH;
bool Settings::force_noscale = false;
