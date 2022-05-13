#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <cmath>
#include <string>
#include <math.h>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

using namespace std;

// each movie data structure has 3 components--id, name, and tags 
struct movie
{
    int id;
    char name[200];
    char tags[200];
};

//each user rating record has 3 components--user id, movie id, and rating
// struct movieRating
// {
//     int userid;
//     int movieid;
//     int rating;
// };

char *movieFileptr = "movies.dat";
char *ratingsFileptr = "ratings.dat";


int movies[4000];
int UserRatings[6050][4000]={0};

int numofMovies = 0;
int numofUsers = 0;
int numofRatings = 0;

// 4000 movies and 1000300 ratings in total
struct movie mData[4000];
// struct movieRating rData[1000300];

double movieSimilarity[4000][4000] = {0};

// function to read movie information
int read_movies()
{
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

    cout<< "number of movies read: "<< numofMovies<<"\n";
    movieFile.close();
    return 1;
}

// function to read ratings
int read_ratings()
{
    // create file stream
    char readline[100];
    ifstream ratingsFile(ratingsFileptr, ios::in);
    
    // check if the file exists
    if (!ratingsFile.is_open())
    {
        printf("Ratings file does not exist!");
        return -1;
    }

    int lastID = -1;
    numofRatings = 0;
    numofUsers = 0;
    int time;
    int userid;
    int movieid;
    int rating;
    
    // read in rating file line by line
    while (ratingsFile.getline(readline, 100))
    {
        sscanf(readline, "%d::%d::%d::%d", &userid, &movieid, &rating, &time);
        UserRatings[userid-1][movieid-1] = rating;
        if (lastID != userid)
        {
            lastID = userid;
            numofUsers++;
        }
        numofRatings++;
    }
    
    cout<< "number of ratings read: " << numofRatings << " number of Users: " << numofUsers << "\n";
    ratingsFile.close();
    return 1;
}


double computeSimilarity(int v1, int v2) {
    int nUsers = 0;
    double dotprod = 0;
    double v1_sq = 0;
    double v2_sq = 0;
    
    for (int i = 0; i < numofUsers; i++) {
        if ((UserRatings[i][v1] > 0) && (UserRatings[i][v2] > 0)) {
            nUsers++;
            dotprod += UserRatings[i][v1] * UserRatings[i][v2];
            v1_sq += UserRatings[i][v1] ^ 2;
            v2_sq += UserRatings[i][v2] ^ 2;
        }
    }

    // if nUsers = 1, movie similarity = 1, meaningless measure
    if ((nUsers <= 1) || (v1_sq == 0) || (v2_sq == 0)) {
        return 0.0;
    }
    
    return dotprod / (sqrt(v1_sq) * sqrt(v2_sq));
}

void similarity_matrix_launcher() {
    for (int v1 = 0; v1 < numofMovies; v1++) {
        for (int v2 = 0; v2 < v1; v2++) {
            movieSimilarity[v1][v2] = computeSimilarity(v1, v2);
            movieSimilarity[v2][v1] = movieSimilarity[v1][v2];
        }
    }
}

void merge(int movieID[], double weight[], int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;
    
    // partition the movieID array
    // maintain two arrays at the same time to keep track of the movieID while sorting on weight
    int L[n1], R[n2];
    int P[n1], Q[n2];
    for (int i = 0; i < n1; i++) {
        L[i] = weight[l + i];
        P[i] = movieID[l + i];
    }
    for (int j = 0; j < n2; j++) {
        R[j] = weight[m + 1 + j];
        Q[j] = movieID[m + 1 + j];
    }
    
    int i = 0;
    int j = 0;
    int k = l;
    while (i < n1 && j < n2) {
        if (L[i] >= R[j]) {
            weight[k] = L[i];
            movieID[k] = P[i];
            i++;
        } else {
            weight[k] = R[j];
            movieID[k] = Q[j];
            j++;
        }
        k++;
    }
    
    // running out of elements in either half
    while (i < n1) {
        weight[k] = L[i];
        movieID[k] = P[i];
        i++;
        k++;
    }
    
    while (j < n2) {
        weight[k] = R[j];
        movieID[k] = Q[j];
        j++;
        k++;
    }
}

void mergesort(int movieID[], double weight[], int l, int r) {
    if (l >= r) {
        return;
    }
    int m = l + (r - l) / 2;
    mergesort(movieID, weight, l, m);
    mergesort(movieID, weight, m + 1, r);
    merge(movieID, weight, l, m, r);
}

int *extract_top(int sorted[], int top) {
    int *arr = new int[top];
    for (int i = 0; i < top; i++) {
        arr[i] = sorted[i];
    }
    return arr;
}

int main()
{
    clock_t t1 = clock();
    read_movies();
    read_ratings();
    clock_t t2 = clock();
    cout<<"elapsed time for file input: "<<(double)(t2-t1)/CLOCKS_PER_SEC<<"\n";

    int topnum = 5;
    int user = 3245;

    clock_t t3 = clock();
    similarity_matrix_launcher();
    clock_t t4 = clock();
    cout<<"elapsed time for computing similarity: "<<(double)(t4-t3)/CLOCKS_PER_SEC<<"\n";

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

    clock_t t5 = clock();
    mergesort(movieIdentifier, weight, 0, numofMovies - 1);
    clock_t t6 = clock();
    cout<<"elapsed time for sorting"<<(double)(t6-t5)/CLOCKS_PER_SEC<<"\n";

    int *result;
    result = extract_top(movieIdentifier, topnum);
    printf("Recommendation for user %d\n", user);
    printf(" Rank | Movie Name\n");
    for (int m1 = 0; m1 < topnum; ++m1)
    {
        printf(" %4d | %s\n", m1 + 1, mData[movies[result[m1]]].name);
    }

    return 0;
}
