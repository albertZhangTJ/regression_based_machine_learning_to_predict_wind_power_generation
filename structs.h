#ifndef STRUCTS_H
#define STRUCTS_H
#include <vector>

struct Test{
    std::vector<std::vector<float>> test;
    std::vector<float> ans;
};

struct point{
    float x,y;
};

struct trial_log{
    std::vector<int> weight;
    int step;
    int init_step;
    int exp;
    bool isActive;
    double avg_precision;
};

#endif //STRUCTS_H