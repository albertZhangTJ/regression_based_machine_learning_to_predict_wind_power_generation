#include "dimension.h"
#include <cmath>
#include <math.h>

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