# regression_based_machine_learning_to_predict_wind_power_generation

This project is coded by me in collaboration with Shengqi Wu. It is a model to perform regression based machine learning (both active learning and random-selection) on the data set https://www.kaggle.com/berkerisen/wind-turbine-scada-dataset.

The project can self-adjust the parameters to achieve the best precision (determined by the mean of absolute value of residual errors). Multithreading is used for faster training.

## compiling
g++ -std=c++11 -pthread *.cpp -o model -static

compiling with qmake with -pthread flag might provide better performance

## running
Download the data set (T1.csv) from kaggle and put it into the same directory as the executable file, run the executable.
