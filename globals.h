#pragma once
#include <fstream>
#include <vector>
#include <random>
#include <mutex>
#include "structs.h"

extern std::mutex lck;
extern int ginit_step;
extern int gstep;
extern std::vector<int> gweight;
extern int gexp;
extern std::random_device rd;
extern std::ofstream dout;
extern std::vector<trial_log> trials;
extern bool gIsActiveOptimization;
