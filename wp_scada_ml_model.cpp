#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <math.h>
#include <cmath>
#include <stdlib.h>
#include <random>
#include <thread>
#include <mutex>
using namespace std;

mutex lck;
int ginit_step;
int gstep;
vector<int> gweight;
int gexp;
random_device rd;
ofstream dout("params.txt");

struct Test{
    vector<vector<float>> test;
    vector<float> ans;
};

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
    data->clear();
    for (vector<float> datum: *test_data){
        used_data->push_back(datum);
    }
    test_data->clear();
    data=used_data;
    this->used_data=new vector<vector<float>>();
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
    this->data->clear();
    this->used_data->clear();
    this->test_data->clear();
}


struct point{
    float x,y;
};


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

dimension::dimension(string name, int exp){
    this->name=name;
    this->exp=exp;
    this->params=new vector<double>(exp,0);
    this->data=new vector<point>();
    this->l_bndry=2100000000;
    this->r_bndry=-1000;
}

void dimension::on_update(vector<point> new_data){
    for (point content: new_data){
        this->data->push_back(content);
        if (content.x>=r_bndry){
            r_bndry=content.x;
        }
        if (content.x<=l_bndry){
            l_bndry=content.x;
        }
    }
    vector<vector<float>> linear_matrix;
    for (size_t i=0;i<=exp;i++){
        vector<float> toAdd;
        for (size_t j=0;j<=exp;j++){
            toAdd.push_back(0);
        }
        linear_matrix.push_back(toAdd);
    }
    //construct a matrix to calculate least-square regression
    //the matrix stores the parameters for partial derivatives
    for (point p: *data){
        for (size_t i=0;i<=exp;i++){
            for (size_t j=0;j<=exp+1;j++){
                if (j==exp+1){
                    linear_matrix[i][j]+=pow(p.x,exp-i)*p.y;
                }
                else {
                    linear_matrix[i][j]+=pow(p.x,2*exp-i-j);
                }
            }
        }
    }
    //use linear programming to solve for the parameters at stationary point
    for (int i=0;i<=exp;i++){
        double divisor=linear_matrix[i][i];
        for (int j=i;j<=exp+1;j++){
            linear_matrix[i][j]/=divisor;
        }
        for (int j=i+1;j<=exp;j++){
            double multiple=linear_matrix[j][i];
            for (int k=i;k<=exp+1;k++){
                linear_matrix[j][k]-=multiple*linear_matrix[i][k];
            }
        }
    }
    for (int i=exp;i>=0;i--){
        for (int j=i-1;j>=0;j--){
            double multi=linear_matrix[j][i];
            linear_matrix[j][i]=0;
            linear_matrix[j][exp+1]-=multi*linear_matrix[i][exp+1];
        }
    }
    params->clear();
    for (int i=exp;i>=0;i--){
        params->push_back(linear_matrix[i][exp+1]);
    }
}

vector<float> dimension::update_bndry(){
    double deriv_abs_max=-1;
    double x=-1;
    for (double i=l_bndry;i<=r_bndry;i+=0.001){
        double v_l=0;
        double v_r=0;
        int exp=0;
        for (double p: *params){
            v_l+=pow(i-0.0005,exp)*p;
            v_r+=pow(i+0.0005,exp)*p;
            exp++;
        }
        double derive_abs=abs((v_r-v_l)/0.001);
        if (deriv_abs_max<derive_abs){
            deriv_abs_max=derive_abs;
            x=i;
        }
    }
    vector<float> ans;
    ans.push_back(x-(r_bndry-l_bndry)/20);
    ans.push_back(x+(r_bndry-l_bndry)/20);
    return ans;
}

double dimension::estimate(float x){
    double ans=0;
    for (size_t i=0;i<params->size();i++){
        ans+=(pow(x,i)*(*params)[i]);
    }
    return ans;
}

dimension::~dimension(){
    this->params->clear();
    this->data->clear();
}


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

model::~model(){
    this->dims->clear();
}

bool gIsActiveOptimization;

void test(model mod, CSVReader src){
    string out_file="";
    if (gIsActiveOptimization){
        if (mod.isActive){
            out_file="act_result.txt";
        }
        else {
            out_file="rnd_result.txt";
        }
    }
    else {
        if (mod.isActive){
            out_file="act_result_r.txt";
        }
        else {
            out_file="rnd_result_r.txt";
        }
    }
    ofstream out(out_file);
    for (size_t i=ginit_step;i<src.data->size()-200;i+=gstep){
        vector<float> residual;
        mod.train(gstep);
        Test t=src.getTestData();
        for (size_t i=0;i<t.test.size();i++){
            residual.push_back(t.ans[i]-mod.estimate(t.test[i]));
        }
        float res_sum=0,abs_sum=0;
        for (float f: residual){
            res_sum+=f;
            abs_sum+=abs(f);
        }
        out<<i<<"    "<<res_sum/t.test.size()<<"    "<<abs_sum/t.test.size()<<"    "<<endl;
    }
}

struct trial_log{
    vector<int> weight;
    int step;
    int init_step;
    int exp;
    bool isActive;
    double avg_precision;
};

vector<trial_log> trials;

bool increment(vector<int>& src){
    src[0]+=1;
    for (int i=0;i<2;i++){
        if (src[i]>4){
            src[i]-=4;
            src[i+1]+=1;
        }
        else {
            break;
        }
    }
    if (src[2] ==5 ){
        return false;
    }
    return true;
}

void adjust_param(int step, int init_step, int exp, CSVReader *csvr){
    vector<int> weight={0,1,1,1};
    while (increment(weight)){
        model mod(csvr,exp);
        mod.initialize(init_step);
        for (int i=init_step;i<csvr->data->size()-400;i+=step){
            mod.train(step);
        }
        vector<float> residual;
        Test t=csvr->getTestData();
        for (size_t i=0;i<t.test.size();i++){
            residual.push_back(t.ans[i]-mod.estimate(t.test[i],weight));
        }
        float res_sum=0,abs_sum=0;
        for (float f: residual){
            res_sum+=f;
            abs_sum+=abs(f);
        }
        trial_log log;
        log.exp=exp;
        log.weight=weight;
        log.init_step=init_step;
        log.isActive=true;
        log.step=step;
        log.avg_precision=res_sum/t.test.size();
        csvr->recycle();
        lck.lock();
        cout<<"Model tested:"<<endl<<"    exp="<<exp<<endl<<"    init_step="<<init_step<<endl<<"    step="<<step<<endl;
        cout<<"    weight="<<weight[0]<<weight[1]<<weight[2]<<weight[3]<<endl<<"    isActive="<<log.isActive<<endl;
        cout<<"    avg_loss="<<log.avg_precision<<endl;
        dout<<"Model tested:"<<endl<<"    exp="<<exp<<endl<<"    init_step="<<init_step<<endl<<"    step="<<step<<endl;
        dout<<"    weight="<<weight[0]<<weight[1]<<weight[2]<<weight[3]<<endl<<"    isActive="<<log.isActive<<endl;
        dout<<"    avg_loss="<<log.avg_precision<<endl;
        trials.push_back(log);
        lck.unlock();
    }
    weight={0,1,1,1};
    while (increment(weight)){
        model mod(csvr,exp);
        mod.setRandom();
        mod.initialize(init_step);
        for (int i=init_step;i<csvr->data->size()-400;i+=step){
            mod.train(step);
        }
        vector<float> residual;
        Test t=csvr->getTestData();
        for (size_t i=0;i<t.test.size();i++){
            residual.push_back(t.ans[i]-mod.estimate(t.test[i],weight));
        }
        float res_sum=0,abs_sum=0;
        for (float f: residual){
            res_sum+=f;
            abs_sum+=abs(f);
        }
        trial_log log;
        log.exp=exp;
        log.weight=weight;
        log.init_step=init_step;
        log.isActive=false;
        log.step=step;
        log.avg_precision=res_sum/t.test.size();
        csvr->recycle();
        lck.lock();
        cout<<"Model tested:"<<endl<<"    exp="<<exp<<endl<<"    init_step="<<init_step<<endl<<"    step="<<step<<endl;
        cout<<"    weight="<<weight[0]<<weight[1]<<weight[2]<<weight[3]<<endl<<"    isActive="<<log.isActive<<endl;
        cout<<"    avg_loss="<<log.avg_precision<<endl;
        dout<<"Model tested:"<<endl<<"    exp="<<exp<<endl<<"    init_step="<<init_step<<endl<<"    step="<<step<<endl;
        dout<<"    weight="<<weight[0]<<weight[1]<<weight[2]<<weight[3]<<endl<<"    isActive="<<log.isActive<<endl;
        dout<<"    avg_loss="<<log.avg_precision<<endl;
        trials.push_back(log);
        lck.unlock();
    }
    delete csvr;
}


int main(){
    vector<string> filepath={"T1.csv"};
    vector<int> sz={10,25,50,100};
    vector<int> isz={500,1000,2000};
    vector<int> exp={2,3,4};
    vector<thread> workers;
    for (int size:sz){
        for (int init_size:isz){
            for (int po: exp){
                CSVReader *csvr=new CSVReader(filepath,0.8);
                thread toAdd(adjust_param, size, init_size, po, csvr);
                workers.push_back(move(toAdd));
            }
        }
    }
    for (auto& th: workers){
        th.join();
    }
    trial_log active_min;
    active_min.avg_precision=100000; //a large enough random number to start with
    trial_log random_min;
    random_min.avg_precision=100000; //same as above
    for (trial_log t: trials){
        if (t.isActive && t.avg_precision<active_min.avg_precision){
            active_min=t;
        }
        else if (!t.isActive && t.avg_precision<random_min.avg_precision){
            random_min=t;
        }
    }
    dout<<endl<<endl;
    gexp=active_min.exp;
    gweight=active_min.weight;
    ginit_step=active_min.init_step;
    gstep=active_min.step;
    gIsActiveOptimization=true;
    dout<<"Active optimization: "<<endl<<"    exp="<<gexp<<endl;
    dout<<"    step="<<gstep<<endl<<"    init_step="<<ginit_step<<endl;
    dout<<"    weight="<<gweight[0]<<gweight[1]<<gweight[2]<<gweight[3]<<endl;
    CSVReader csvr(filepath,0.8);

    model act(&csvr);
    act.initialize(ginit_step);
    test(act,csvr);
    csvr.recycle();

    model rnd(&csvr);
    rnd.setRandom();
    rnd.initialize(ginit_step);
    test(rnd,csvr);
    csvr.recycle();

    gexp=random_min.exp;
    gweight=random_min.weight;
    ginit_step=random_min.init_step;
    gstep=random_min.step;
    gIsActiveOptimization=false;
    dout<<"Random optimization: "<<endl<<"    exp="<<gexp<<endl;
    dout<<"    step="<<gstep<<endl<<"    init_step="<<ginit_step<<endl;
    dout<<"    weight="<<gweight[0]<<gweight[1]<<gweight[2]<<gweight[3]<<endl;

    model sact(&csvr);
    sact.initialize(ginit_step);
    test(sact,csvr);
    csvr.recycle();

    model srnd(&csvr);
    srnd.setRandom();
    srnd.initialize(ginit_step);
    test(srnd,csvr);

    return 0;
}

