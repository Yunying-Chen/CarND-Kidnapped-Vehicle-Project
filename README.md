# CarND-Kidnapped-Vehicle-Project                   

## Intro
In this project, a prticle filter is built to estimate the location of the car. A map and some initial localization information (analogous to what a GPS would provide) are provided. At each time step, observation and control data are also given.


## Dependencies                      
cmake >= 3.5                  
make >= 4.1                       
gcc/g++ >= 5.4                                   

## Files              
* src/particle_filter.cpp —— contains the scaffolding of a ParticleFilter class including initializing, predicting, updating weights and resampling                               
* src/main.cpp ——contains the code that will be running the particle filter

## Run the code
From the build directory, execute the following command line:                        
1. mkdir build && cd build
2. cmake && make
3. ./particle_filter               

## Performance
In this project **100 particles** are used.                              
System Time: 51.76s                                       
| ESTIMATION | ERROR |  
| - | -: | 
| x | 0.116 | 
| y | 0.107 | 
| yaw | 0.004 |                             
![Image text](https://github.com/Yunying-Chen/CarND-Kidnapped-Vehicle-Project/blob/master/img/result.png)    
