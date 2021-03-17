#ifndef CSVREADER_H
#define CSVREADER_H
#include <vector>
#include <string>
#include "structs.h"

using namespace std;
class CSVReader{
    public:
        vector<string> filepath;
        vector<vector<float>>* data; //dynamic
        vector<vector<float>>* used_data;
        vector<vector<float>>* test_data;
        vector<string> fields;
        float percentage;

        CSVReader(vector<string>& filepath, float percentage);
        //load the data into memory
        void initialize();
        //since we only have one input file here, we have to divide it into test data and training data
        void divide(float per);
        /* since the data in the .csv file is sorted by time
         * if we always acquire data started from the beginning
         * it is likely that the model will be affected by time
         * thus the random start
         */
        void randomize();
        vector<vector<float>> getData(string field, float lhs, float rhs, size_t amnt);
        vector<vector<float>> getRandom(size_t amnt);
        void recycle();
        Test getTestData();
        ~CSVReader();
};

#endif //CSVREADER_H