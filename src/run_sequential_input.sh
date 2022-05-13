mpic++ -fopenmp -o movierecommendation_sequential_input movierecommendation_sequential_input.cpp
srun -c1 -n1 ./movierecommendation_sequential_input
srun -c1 -n2 ./movierecommendation_sequential_input
srun -c1 -n4 ./movierecommendation_sequential_input
srun -c1 -n8 ./movierecommendation_sequential_input
srun -c1 -n10 ./movierecommendation_sequential_input
