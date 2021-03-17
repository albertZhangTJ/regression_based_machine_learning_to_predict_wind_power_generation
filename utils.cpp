#include "globals.h"
#include "utils.h"
#include <iostream>
using namespace std;

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
    bool examinedActive=false;
    //test whether a log file for this set of parameters exist
    //if exist, the model is once ran and failed to complete
    //thus we can just move on from where we left off
    string fname="";
    fname=fname+to_string(step)+"_"+to_string(init_step)+"_"+to_string(exp)+".log";
    ifstream lin(fname);
    if (lin.good()){
        int wght;
        lin>>wght;
        //since both the active model above and the random model below uses the same log file
        //0 is used as a flag to tell weight for active models and random models apart
        if (wght<0){
            examinedActive=true;
        }
        wght=abs(wght);
        string wt=to_string(wght);
        weight[0]=stoi(&wt[0]);
        weight[1]=stoi(&wt[1]);
        weight[2]=stoi(&wt[2]);
        weight[3]=stoi(&wt[3]);
    }
    lin.close();
    //training active learning model based the paramters above
    //exhaustively examine through all combinations of weight that are deemed to be appropriate
    while (!examinedActive && increment(weight)){
        //train the model
        model mod(csvr,exp);
        mod.initialize(init_step);
        for (int i=init_step;i<csvr->data->size()-400;i+=step){
            mod.train(step);
        }
        //test the model && add the results to the log
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
        ofstream lout(fname,ofstream::trunc);
        lout<<weight[0]<<weight[1]<<weight[2]<<weight[3]<<flush;
        lout.close();
    }

    //training random-selection model based on the parameters above
    if (!examinedActive){
        weight={0,1,1,1};
    }
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
        ofstream lout(fname,ofstream::trunc);
        lout<<"-"<<weight[0]<<weight[1]<<weight[2]<<weight[3]<<flush;
        lout.close();
    }
    delete csvr;
}