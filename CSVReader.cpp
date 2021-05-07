#include "CSVReader.h"
#include <random>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include "globals.h"
using namespace std;


CSVReader::CSVReader(vector<string>& filepath, float percentage){
    this->filepath=filepath;
    this->percentage=percentage;
    this->test_data=new vector<vector<float>>();
    this->used_data=new vector<vector<float>>();
    this->initialize();
    this->divide(this->percentage);
    this->randomize();
}

void CSVReader::initialize(){
    this->data=new vector<vector<float>>();
//    bool fieldsInitialized=false;
    string line="";
    for (string path: filepath){
        ifstream in(path);
        getline(in,line);
        fields.push_back("month");
        fields.push_back("hour");
        fields.push_back("LV power");
        fields.push_back("ws");
        fields.push_back("theoretical power");
        fields.push_back("wd");
//        if (!fieldsInitialized){
//            string temp="";
//            bool timeSeged=false;
//            for (size_t i=0;i<line.size();i++){
//                //date(actually month) and time-of-day is considered separately
//                if (line[i]==',' || (line[i]=='/'&&!timeSeged)){
//                    if (line[i]=='/'){
//                        timeSeged=true;
//                    }
//                    fields.push_back(temp);
//                    temp="";
//                }
//                else {
//                    temp=temp+line[i];
//                }
//            }
//            fields.push_back(temp);
//            fieldsInitialized=true;
//        }
        while(getline(in,line)){
            vector<float> toAdd;
            string temp="";
            for (size_t i=0;i<line.size();i++){
                if (line[i]==','){
                    //specifically designed for handling time on the first column
                    if (toAdd.size()==0){
                        int cnt=0;
                        string subtemp="";
                        for (size_t j=0;j<temp.size();j++){
                            if (temp[j]==' '){
                                //ditch exact date for simplicity
                                //the whole data set only contains data from 2018, so year is not considered as well
                                if (cnt==1){
                                    //The difference from the month to July (which is usually the hottest month on land in northern hemisphere) is used
                                    toAdd.push_back(abs(7-(float)stod(subtemp)));
                                }
                                cnt++;
                                subtemp="";
                            }
                            if (temp[j]==':'){
                                break;
                            }
                            subtemp=subtemp+temp[j];
                        }
                        //time of day is considered hourly for simplicity
                        toAdd.push_back(stof(subtemp));
                    }
                    else{
                        toAdd.push_back((float)stod(temp));
                        temp="";
                    }
                }
                else {
                    temp=temp+line[i];
                }
            }
            toAdd.push_back((float)stod(temp));
            data->push_back(toAdd);
        }
    }
}

void CSVReader::divide(float per){
    while(data->size()>(data->size()+test_data->size())*per){
        mt19937 mtrand(rd());
        size_t index=mtrand()%data->size();
        test_data->push_back((*data)[index]);
        data->erase(data->begin()+index);
    }
}

void CSVReader::randomize(){
    mt19937 mtrand(rd());
    //why 655350? Absolutely no reason...
    for (size_t i=0;i<655350;i++){
        size_t lhs=mtrand()%data->size();
        size_t rhs=mtrand()%data->size();
        vector<float> temp=(*data)[lhs];
        (*data)[lhs]=(*data)[rhs];
        (*data)[rhs]=temp;
    }
}

vector<vector<float>> CSVReader::getData(string field, float lhs, float rhs, size_t amnt){
    vector<vector<float>> ans;
    size_t index=-1;
    for (size_t i=0;i<fields.size();i++){
        if (fields[i]==field){
            index=i;
            break;
        }
    }
    if (index==(size_t)-1){
        return ans;
    }
    for (size_t i=0;i<data->size();i++){
        if ( (*data)[i][index]>=lhs && (*data)[i][index]<=rhs ){
            ans.push_back((*data)[i]);
            used_data->push_back((*data)[i]);
            data->erase(data->begin()+i);
            i--;
        }
        if (ans.size()==amnt){
            break;
        }
    }
    return ans;
}

vector<vector<float>> CSVReader::getRandom(size_t amnt){
    vector<vector<float>> ans;
    for (size_t i=0; i<amnt && data->size()>0;i++){
        ans.push_back((*data)[0]);
        used_data->push_back((*data)[0]);
        data->erase(data->begin());
    }
    return ans;
}

void CSVReader::recycle(){
    for (vector<float> datum: *data){
        used_data->push_back(datum);
    }
    delete data;
    for (vector<float> datum: *test_data){
        used_data->push_back(datum);
    }
    delete test_data;
    data=used_data;
    this->used_data=new vector<vector<float>>();
    this->test_data=new vector<vector<float>>();
    this->divide(0.8);
    this->randomize();
}

Test CSVReader::getTestData(){
    Test ans;
    for (vector<float> datum: *test_data){
        vector<float> toAdd={datum[0],datum[1],datum[3],datum[5]};
        ans.test.push_back(toAdd);
        ans.ans.push_back(datum[2]);
    }
    return ans;
}

CSVReader::~CSVReader(){
    delete this->data;
    delete this->used_data;
    delete this->test_data;
}