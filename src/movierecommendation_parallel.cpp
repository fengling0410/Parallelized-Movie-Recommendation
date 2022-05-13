#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <cmath>
#include <math.h>
#include <cstdlib>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <memory.h>
#include <mpi.h>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;

/*
To compile this file -> mpic++ -fopenmp movierecommendation_parapllel.cpp
To run this program -> srun -c1 -n20 ./a.out

Run time options:
    -n: how many processors to use. The larger n, the shorter runtime. 
    -c: how many cpu per processor

Data of this program is the MovieLens 1M dataset from http://grouplens.org/datasets/movielens/
*/

// each movie data structure has 3 components--id, name, and tags 
struct movie
{
    int id;
    char name[200];
    char tags[200];
};


int UserRatings[6050][4000] = {0};
int numofMovies = 0;
int numofRatings = 0;
int numofUsers = 0;
int movies[4000];

// 4000 movies and 1000300 ratings in total
struct movie mData[4000];
// struct movieRating rData[1000300];

double movieSimilarity[4000][4000]={0.0};

// function to read movie information
int read_movies()
{
    char* movieFileptr = "movies.dat";
    //create file stream
    ifstream movieFile(movieFileptr, ios::in);

    // check if the file exists
    if (!movieFile.is_open())
    {
        printf("Movies file does not exist!");
        return -1;
    }

    char readline[256];
    numofMovies = 0;

    // read in movie file line by line
    while (movieFile.getline(readline, 255))
    {
        sscanf(readline, "%d::%[^::]::%s", &mData[numofMovies].id, mData[numofMovies].name, mData[numofMovies].tags);
        movies[mData[numofMovies].id] = numofMovies;
        numofMovies++;
    }

    movieFile.close();
    return 1;
}

// function to read ratings
int read_ratings()
{
    int rank;
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    

    int lastID = -1;
    int numofRatings_local = 0;
    int numofUsers_local = 0;
    int time;
    int userid;
    int movieid;
    int rating;

    char* file_name[10] = { "ratingsaa.dat",  "ratingsab.dat",  "ratingsac.dat",  "ratingsad.dat",  "ratingsae.dat",  "ratingsaf.dat",
     "ratingsag.dat",  "ratingsah.dat",  "ratingsai.dat",  "ratingsaj.dat"};

     for(int i=0;i<10;i++)
     {
         if(rank == i)
         {
            char *ratingsFileptr = file_name[i];
            char readline[100];
            ifstream ratingsFile(ratingsFileptr, ios::in);
            // check if the file exists
            if (!ratingsFile.is_open())
            {
                printf("Ratings file does not exist!");
                return -1;
            }

            while (ratingsFile.getline(readline, 100))
            {
                sscanf(readline, "%d::%d::%d::%d", &userid, &movieid, &rating, &time);
                UserRatings[userid-1][movieid-1] = rating;
                if (lastID != userid)
                {
                    lastID = userid;
                    numofUsers_local++;
                }
                numofRatings_local++;
            }

            // cout<< "current rank: "<<rank<<"number of ratings read: " << numofRatings_local << " number of Users: " << numofUsers_local << "\n";
            ratingsFile.close();
         }
    }

    MPI_Allreduce(MPI_IN_PLACE, &UserRatings, 6050 * 4000, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&numofRatings_local, &numofRatings, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&numofUsers_local, &numofUsers, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    numofUsers = numofUsers -9;
    return 1;
}


double computeMovieSimilarity(int m1, int m2)
{
    int numR = 0;
    double dot = 0;
    double denom_x = 0;
    double denom_y = 0;

    //printf("in computeMovieSimilarity %d %d\n", m1, m2);
    #pragma omp parallel for reduction(+:numR, dot, denom_x, denom_y) num_threads(4)
    for (int i = 0; i < numofUsers; ++i)
    {
        // only compute for positive values (ratings are always positive)
        // ignore zero values that is missing movieRating from the user 'i' for the movie m1 or m2
        if ((UserRatings[i][m1] > 0) && (UserRatings[i][m2] > 0))
        {
            numR++;
            dot += UserRatings[i][m1] * UserRatings[i][m2];
            denom_x += UserRatings[i][m1] ^ 2;
            denom_y += UserRatings[i][m2] ^ 2;
        }
    }

    if ((numR <= 1) || (denom_x == 0) || (denom_y == 0))
    {
        return 0.0;
    }
    return dot / (sqrt(denom_x) * sqrt(denom_y));
}



void movie_similarity()
{
    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    for (int m1 = 0; m1 < numofMovies; m1++)
    {
        for (int m2 = 0; m2 < m1; m2++)
        {
            // allocate to different processes as per rank
            if ((m1 + m2) % size == rank)
            {
                movieSimilarity[m1][m2] = computeMovieSimilarity(m1, m2);
                movieSimilarity[m2][m1] = movieSimilarity[m1][m2];
            }
        }
    }

    MPI_Allreduce(MPI_IN_PLACE, &movieSimilarity, 4000 * 4000, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

}

bool eq(double a, double b, double epsilon) {
    return abs(a - b) < epsilon;
}

bool le(double a, double b, double epsilon) {
    return a < b + epsilon;
}
bool gr(double a, double b, double epsilon) {
    return a > b - epsilon;
}

void merge(vector< pair<double, int> >& v, int s, int m, int e){
    vector< pair<double, int> > temp;
    int i = s, j = m + 1;
    while(i <= m && j <= e)
    {
        if(gr(v[i].first, v[j].first, 1e-9)){
            temp.push_back(v[i]);
            ++i;
        }
        else{
            temp.push_back(v[j]);
            ++j;
        }
    }
    while(i <= m){
        temp.push_back(v[i]);
        ++i;
    }
    while(j <= e){
        temp.push_back(v[j]);
        ++j;
    }
    for (int i = s; i <= e; ++i) {
        v[i] = temp[i - s];
    }
}

void mergesort(vector< pair<double, int> >& weight, int left, int right){
    if (left < right && (right - left > 1000)){
        int middle = (left + right)/2;
        #pragma omp parallel num_threads(2)
        {
            #pragma omp single
            {
                #pragma omp task
                mergesort(weight,left,middle);
                #pragma omp task
                mergesort(weight,middle+1,right);
            }
        }
        merge(weight,left,middle,right);
    } else if (left < right){
        int middle = (left + right)/2;
        mergesort(weight, left, middle);
        mergesort(weight, middle+1, right);
        merge(weight, left, middle, right);
    }
}

int *extract_top(vector< pair<double, int> >& vec, int top) {
    int *arr = new int[top];
    for (int i = 0; i < top; i++) {
        arr[i] = vec[i].second;
    }
    return arr;
}

int main(int argc, char *argv[])
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if(rank==0)
    {
        cout<<"number of processors: "<<size<<"\n";
    }
    
    double t1, t2; 
    t1 = MPI_Wtime();
    read_movies();
    read_ratings();
    t2 = MPI_Wtime();
    if(rank == 0){cout<< "number of movies read: "<< numofMovies<<"\n";}
    if(rank == 0){cout<<"number of ratings read: " <<numofRatings<<"\n";cout<<" number of users read: "<<numofUsers<<"\n";}
    if(rank == 0){cout<<"elapsed time for file input: "<<t2-t1<<"\n";}

    double t3, t4;
    t3 = MPI_Wtime();
    movie_similarity();
    t4 = MPI_Wtime();
    if(rank == 0){cout<<"elapsed time for computing similarity: "<<t4-t3<<"\n";}

    int topnum = 5;
    int user = 3245;

    if (rank == 0) {
        int movieIdentifier[numofMovies];
        double weight[numofMovies];
        for (int m1 = 0; m1 < numofMovies; ++m1)
        {
            movieIdentifier[m1] = mData[m1].id;
            weight[m1] = 0;
            for (int m2 = 0; m2 < numofMovies; ++m2)
            {
                weight[m1] += (UserRatings[user - 1][m2] * movieSimilarity[m1][m2]);
            }
        }
        
        vector< pair<double, int> > vec;
        typedef pair<double, int> my_pair;
        for (int i = 0; i < numofMovies; i++) {
            const my_pair p = make_pair(weight[i], movieIdentifier[i]);
            vec.push_back(p);
        }
        
        double t5, t6;
        t5 = MPI_Wtime();
        mergesort(vec, 0, numofMovies - 1);
        t6 = MPI_Wtime();
        cout<<"elapsed time for sorting: "<<t6-t5<<"\n";
        
        int *result;
        result = extract_top(vec, topnum);

        printf("Recommendation for user %d\n", user);
        printf(" Rank | Movie Name\n");
        for (int m1 = 0; m1 < topnum; ++m1)
        {
            printf(" %4d | %s\n", m1 + 1, mData[movies[result[m1]]].name);
        }
    }

    return 0;
}
