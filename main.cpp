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
#include <pthread.h>
#include <mutex>
#include "CSVReader.h"
#include "structs.h"
#include "model.h"
#include "dimension.h"
#include "utils.h"
#include "globals.h"
using namespace std;


int main(){
    vector<string> filepath={"T1.csv"};
    
    vector<int> sz={100,50,25,10};
    vector<int> isz={2000,1000,500};
    vector<int> exp={4,3,2};
    vector<thread> workers;
    for (int size:sz){
        for (int init_size:isz){
            for (int po: exp){
                //actually the desktop we plan to train this model on only supports 8 threads
                //thus it might seems more reasonable to use at most 16 threads
                //but I didn't do so for code simplicity (although a thread pool seems neat)
                CSVReader *csvr=new CSVReader(filepath,0.8);
                thread toAdd(adjust_param, size, init_size, po, csvr);
                workers.push_back(move(toAdd));
            }
        }
    }
    for (auto& th: workers){
        th.join();
    }

    get_past_results();
    trial_log active_min;
    active_min.avg_precision=100000; //a large enough random number to start with
    trial_log random_min;
    random_min.avg_precision=100000; //same as above
    for (trial_log t: trials){
        if (t.isActive && abs(t.avg_precision)<abs(active_min.avg_precision)){
            active_min=t;
        }
        else if (!t.isActive && abs(t.avg_precision)<abs(random_min.avg_precision)){
            random_min=t;
        }
    }
    cout<<endl<<endl;
    gexp=active_min.exp;
    gweight=active_min.weight;
    ginit_step=active_min.init_step;
    gstep=active_min.step;
    gIsActiveOptimization=true;
    cout<<"Active optimization: "<<endl<<"    exp="<<gexp<<endl;
    cout<<"    step="<<gstep<<endl<<"    init_step="<<ginit_step<<endl;
    cout<<"    weight="<<gweight[0]<<gweight[1]<<gweight[2]<<gweight[3]<<endl;
    cout<<flush;

    //save time by skipping disk IO and directly copying from memory
    CSVReader csvr(filepath,0.8);
    CSVReader scsvr=csvr;
    CSVReader tcsvr=csvr;
    CSVReader fcsvr=csvr;
    CSVReader acsvr=csvr;
    CSVReader bcsvr=csvr;
    CSVReader ccsvr=csvr;
    CSVReader dcsvr=csvr;



    model act(&csvr);
    act.initialize(ginit_step);
    cout<<csvr.data->size()<<endl<<flush;
    test(act,csvr);
    cout<<csvr.data->size()<<"    "<<csvr.used_data->size()<<"    "<<csvr.test_data->size()<<endl<<flush;
    cout<<"First model tested"<<endl<<flush;
    cout<<"data recycled"<<endl<<flush;

    model pact(&acsvr);
    act.initialize(ginit_step);
    cout<<acsvr.data->size()<<endl<<flush;
    partial_test(pact,acsvr,2);
    cout<<acsvr.data->size()<<"    "<<acsvr.used_data->size()<<"    "<<acsvr.test_data->size()<<endl<<flush;
    cout<<"First model partial tested"<<endl<<flush;
    cout<<"data recycled"<<endl<<flush;

    model rnd(&scsvr);
    rnd.setRandom();
    rnd.initialize(ginit_step);
    cout<<"second model initialized"<<endl<<flush;
    test(rnd,scsvr);

    model prnd(&bcsvr);
    prnd.setRandom();
    prnd.initialize(ginit_step);
    partial_test(prnd,bcsvr,2);

    gexp=random_min.exp;
    gweight=random_min.weight;
    ginit_step=random_min.init_step;
    gstep=random_min.step;
    gIsActiveOptimization=false;
    cout<<"Random optimization: "<<endl<<"    exp="<<gexp<<endl;
    cout<<"    step="<<gstep<<endl<<"    init_step="<<ginit_step<<endl;
    cout<<"    weight="<<gweight[0]<<gweight[1]<<gweight[2]<<gweight[3]<<endl;
    cout<<flush;
    
    model sact(&tcsvr);
    sact.initialize(ginit_step);
    test(sact,tcsvr);

    model psact(&ccsvr);
    psact.initialize(ginit_step);
    partial_test(psact,ccsvr,2);

    model srnd(&fcsvr);
    srnd.setRandom();
    srnd.initialize(ginit_step);
    test(srnd,fcsvr);

    model psrnd(&dcsvr);
    psrnd.setRandom();
    psrnd.initialize(ginit_step);
    partial_test(psrnd,dcsvr,2);

    return 0;
}

