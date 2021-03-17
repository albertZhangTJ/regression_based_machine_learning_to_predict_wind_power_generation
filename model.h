#ifndef MODEL_H
#define MODEL_H
#include "CSVReader.h"
#include "dimension.h"
#include <vector>
using namespace std;

class model{
    public:
        vector<dimension>* dims;
        bool isActive;
        CSVReader* csvr;

        model(CSVReader* csvr);
        model(CSVReader* csvr, int exp);
        void initialize(size_t size);
        void train(size_t step);
        void setActive();
        void setRandom();
        double estimate(vector<float> features);
        double estimate(vector<float> features, vector<int> weight);
        ~model();
};

#endif //MODEL_H