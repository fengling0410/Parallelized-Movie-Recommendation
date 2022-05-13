# Item-based Movie Recommendation with OpenMP and MPI

## Background and Porject Description
Movie recommendation system has been a very important application of online video compary. The purpose of our project is to build an item-based collaborative filtering movie recommendation model with shared memory parallelism OpenMP and distributed memory parallelism MPI to improve the efficiency of movie recommendation process.
The parallelism in our application mainly contains three parts:
* Parallelized file input using MPI
* Parallelized similarity matrix computation using MPI and OpenMP
* Parallelized merge sort using OpenMP

## Source Code Description
The source code of our application resides in the directory **./src/** with required dataset files.
We provide three c++ files:
* **movierecommendation_serial.cpp:** This is the baseline serial code of our movie recommendation application.
* **movierecommendation_parallel.cpp:** This is the application with parallelized file input, similarity matrix computation, and merge sort(with cross-over implementation).
* **movierecommendation_sequential_input.cpp:** This is the application with parallelized similarity matrix computation and merge sort, but sequential file input (Given current dataset, the parallelized file input doesn't provide too much speedup compared with the serial baseline, therefore this version of code is provided). Here the merge sort does not have cross-over implementation for a better comparison.

## How to Run Our Code
We have provided three bash scripts under the source code directory for running our code and performing experiments with different numbers of processors.
* **run_serial.sh:** This script will run the movierecommendation_serial.cpp
* **run_parallel.sh:** This script will run the movierecommendation_parallel.cpp with number of processors = 10
* **run_sequential_input:** This script will run the movierecommendation_sequential_input.cpp with number of processors = 1, 2, 4, 8, 10

To run the bash script or to run our code directly, please checkout a Broadwell node and load the required module using the following commands first:
```
salloc -N1 -c32 -p broadwell -t 01:00:00
module load gcc/10.2.0-fasrc01 openmpi/4.1.1-fasrc01
```

You can use the bash script to run the code and experiments:
```
bash run_sequential_input.sh
```

If you want to run the code without using bash script, you can do:
```
mpic++ -fopenmp -o movierecommendation_parallel movierecommendation_parallel.cpp
srun -c1 -n10 ./movierecommendation_parallel
```

For the experiment result, roofline analysis, and strong scaling analysis, please refer to our report.