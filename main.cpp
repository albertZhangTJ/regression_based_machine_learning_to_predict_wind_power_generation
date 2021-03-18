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
#include "CSVReader.h"
#include "structs.h"
#include "model.h"
#include "dimension.h"
#include "utils.h"
#include "globals.h"
using namespace std;


int main(){
    vector<string> filepath={"T1.csv"};
    get_past_results();
    vector<int> sz={10,25,50,100};
    vector<int> isz={500,1000,2000};
    vector<int> exp={2,3,4};
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

