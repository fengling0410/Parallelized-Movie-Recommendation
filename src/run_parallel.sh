mpic++ -fopenmp -o movierecommendation_parallel movierecommendation_parallel.cpp
srun -c1 -n10 ./movierecommendation_parallel
