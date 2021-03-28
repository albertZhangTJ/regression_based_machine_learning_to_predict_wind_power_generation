#include "model.h"
#include "globals.h"

model::model(CSVReader* csvr){
    this->csvr=csvr;
    this->dims=new vector<dimension>();
    for (size_t i=0;i<csvr->fields.size();i++){
        dimension to_push(csvr->fields[i],gexp);
        this->dims->push_back(to_push);
        //the live power production is the dependent variables
        //the theoretical power production is determined by wind speed, thus is not an independent variable as well
        if (i==2 || i==4){
            dims->pop_back();
        }
    }
    this->isActive=true;
}

model::model(CSVReader* csvr,int exp){
    this->csvr=csvr;
    this->dims=new vector<dimension>();
    for (size_t i=0;i<csvr->fields.size();i++){
        dimension to_push(csvr->fields[i],exp);
        this->dims->push_back(to_push);
        //the live power production is the dependent variables
        //the theoretical power production is determined by wind speed, thus is not an independent variable as well
        if (i==2 || i==4){
            dims->pop_back();
        }
    }
    this->isActive=true;
}

void model::initialize(size_t size){
    vector<vector<float>> data=this->csvr->getRandom(size);
    vector<point> hrs;
    vector<point> mnth;
    vector<point> ws;
    vector<point> wd;
    for (vector<float> datum: data){
        if (datum.size()!=6){
            continue;
        }
        point p={datum[0],datum[2]};
        mnth.push_back(p);
        p={datum[1],datum[2]};
        hrs.push_back(p);
        p={datum[3],datum[2]};
        ws.push_back(p);
        p={datum[5],datum[2]};
        wd.push_back(p);
    }
    (*dims)[0].on_update(mnth);
    (*dims)[1].on_update(hrs);
    (*dims)[2].on_update(ws);
    (*dims)[3].on_update(wd);
}

void model::train(size_t step){
    if (isActive){
        for (size_t i=0;i<dims->size();i++){
            vector<float> intvl=(*dims)[i].update_bndry();
            vector<vector<float>> data=this->csvr->getData((*dims)[i].name,intvl[0],intvl[1],step/dims->size());
            if (data.size()==0){
                data=this->csvr->getRandom(step);
            }
            vector<point> hrs, mnth, ws, wd;
            for (vector<float> datum: data){
                if (datum.size()!=6){
                    continue;
                }
                mnth.push_back({datum[0],datum[2]});
                hrs.push_back({datum[1],datum[2]});
                ws.push_back({datum[3],datum[2]});
                wd.push_back({datum[5],datum[2]});
            }
            (*dims)[0].on_update(mnth);
            (*dims)[1].on_update(hrs);
            (*dims)[2].on_update(ws);
            (*dims)[3].on_update(wd);
        }
    }
    else {
        initialize(step);
    }

}

void model::setActive(){
    this->isActive=true;
}

void model::setRandom(){
    this->isActive=false;
}

double model::estimate(vector<float> features){
    double ans_sum=0;
    int weight_sum=0;
    for (size_t i=0;i<dims->size();i++){
        ans_sum+=(*dims)[i].estimate(features[i])*gweight[i];
        weight_sum+=gweight[i];
    }
    return ans_sum/weight_sum;
}

double model::estimate(vector<float> features, vector<int> weight){
    double ans_sum=0;
    int weight_sum=0;
    for (size_t i=0;i<dims->size();i++){
        ans_sum+=(*dims)[i].estimate(features[i])*weight[i];
        weight_sum+=weight[i];
    }
    return ans_sum/weight_sum;
}

double model::partial_estimate(vector<float> feature, vector<int> weight){
    double ans_sum=0;
    int weight_sum=0;
    for (size_t i=0;i<dims->size();i++){
        if (weight[i]>0){
            ans_sum+=(*dims)[i].estimate(feature[i])*weight[i];
            weight_sum+=weight[i];
        }
    }
    return ans_sum/weight_sum;
}

model::~model(){
    this->dims->clear();
}