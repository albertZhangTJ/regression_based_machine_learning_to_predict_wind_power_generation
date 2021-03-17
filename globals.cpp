#include <fstream>
#include <vector>
#include <random>
#include <mutex>
#include "globals.h"
#include "structs.h"

std::mutex lck;
int ginit_step;
int gstep;
std::vector<int> gweight;
int gexp;
std::random_device rd;
std::ofstream dout("params.txt",std::ofstream::app);
std::vector<trial_log> trials;
bool gIsActiveOptimization;