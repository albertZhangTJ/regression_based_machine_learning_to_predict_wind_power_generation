#ifndef UTIL_H
#define UTIL_H
#include <vector>
#include "CSVReader.h"
#include "model.h"
using namespace std;

void test(model mod, CSVReader src);
bool increment(vector<int>& src);
void adjust_param(int step, int init_step, int exp, CSVReader *csvr);

#endif