#ifndef DIMENSION_H
#define DIMENSION_H
#include <vector>
#include <string>
#include "structs.h"
using namespace std;


class dimension{
    public:
        int exp;
        string name;
        vector<double>* params;
        vector<point>* data;
        float l_bndry;
        float r_bndry;

        dimension(string name, int exp);
        void on_update(vector<point> new_data);
        //the active learning boundary is determined using the largest absolute value for derivative
        //since here a minimal shift in x leads to large change in y, thus worth more of our attention
        vector<float> update_bndry();
        double estimate(float x);
        ~dimension();
};

#endif