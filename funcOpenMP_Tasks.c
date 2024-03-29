#include <sys/time.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>

/* I/O routines */
FILE *open_traindata(char *trainfile)
{
	FILE *fp;

	fp = fopen(trainfile, "r");
	if (fp == NULL) {
		printf("traindata; File %s not available\n", trainfile);
		exit(1);
	}
	return fp;
}

FILE *open_querydata(char *queryfile)
{
	FILE *fp;

	fp = fopen(queryfile, "r");
	if (fp == NULL) {
		printf("querydata: File %s not available\n", queryfile);
		exit(1);
	}
	return fp;
}

double read_nextnum(FILE *fp)
{
	double val;

	int c = fscanf(fp, "%lf", &val);
	if (c <= 0) {
		fprintf(stderr, "fscanf returned %d\n", c);
		exit(1);
	}
	return val;
}

/* Timer */
double gettime()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (double) (tv.tv_sec+tv.tv_usec/1000000.0);
}

/* Function to approximate */
double fitfun(double *x, int n)
{
	double f = 0.0;
	int i;

#if 1
	for(i=0; i<n; i++)	/* circle */
		f += x[i]*x[i];
#endif
#if 0
	for(i=0; i<n-1; i++) {	/*  himmelblau */
		f = f + pow((x[i]*x[i]+x[i+1]-11.0),2) + pow((x[i]+x[i+1]*x[i+1]-7.0),2);
	}
#endif
#if 0
	for (i=0; i<n-1; i++)   /* rosenbrock */
		f = f + 100.0*pow((x[i+1]-x[i]*x[i]),2) + pow((x[i]-1.0),2);
#endif
#if 0
	for (i=0; i<n; i++)     /* rastrigin */
		f = f + pow(x[i],2) + 10.0 - 10.0*cos(2*M_PI*x[i]);
#endif

	return f;
}


/* random number generator  */
#define SEED_RAND()     srand48(1)
#define URAND()         drand48()

#ifndef LB
#define LB -1.0
#endif
#ifndef UB
#define UB 1.0
#endif

double get_rand(int k)
{
	return (UB-LB)*URAND()+LB;
}


/* utils */
double compute_min(double *v, int n)
{
	int i;
	double vmin = v[0];
	for (i = 1; i < n; i++)
		if (v[i] < vmin) vmin = v[i];

	return vmin;
}

double compute_max(double *v, int n)
{
	int i;
	double vmax = v[0];
	for (i = 1; i < n; i++)
		if (v[i] > vmax) vmax = v[i];

	return vmax;
}

double compute_sum(double *v, int n)
{
	int i;
	double s = 0;
	for (i = 0; i < n; i++) s += v[i];

	return s;
}

double compute_sum_pow(double *v, int n, int p)
{
	int i;
	double s = 0;
	for (i = 0; i < n; i++) s += pow(v[i], p);

	return s;
}

double compute_mean(double *v, int n)
{
	int i;
	double s = 0;
	for (i = 0; i < n; i++) s += v[i];

	return s/n;
}

double compute_std(double *v, int n, double mean)
{
	int i;
	double s = 0;
	for (i = 0; i < n; i++) s += pow(v[i]-mean,2);

	return sqrt(s/(n-1));
}

double compute_var(double *v, int n, double mean)
{
	int i;
	double s = 0;
	for (i = 0; i < n; i++) s += pow(v[i]-mean,2);

	return s/n;
}

double compute_dist(double *v, double *w, int n)
{
	int i;
	double s = 0.0;
	for (i = 0; i < n; i++) {
		s+= pow(v[i]-w[i],2);
	}

	return sqrt(s);
}

double compute_max_pos(double *v, int n, int *pos)
{
	int i, p = 0;
	double vmax = v[0];
	for (i = 1; i < n; i++)
		if (v[i] > vmax) {
			vmax = v[i];
			p = i;
		}

	*pos = p;
	return vmax;
}

double compute_min_pos(double *v, int n, int *pos)
{
	int i, p = 0;
	double vmin = v[0];
	for (i = 1; i < n; i++)
		if (v[i] < vmin) {
			vmin = v[i];
			p = i;
		}

	*pos = p;
	return vmin;
}

double compute_root(double dist, int norm)
{
	if (dist == 0) return 0;

	switch (norm) {
	case 2:
		return sqrt(dist);
	case 1:
	case 0:
		return dist;
	default:
		return pow(dist, 1 / (double) norm);
	}
}

double compute_distance(double *pat1, double *pat2, int lpat, int norm)
{
	register int i;
	double dist = 0.0;

	for (i = 0; i < lpat; i++) {
		double diff = 0.0;

		diff = pat1[i] - pat2[i];

		switch (norm) {
		double   adiff;

		case 2:
			dist += diff * diff;
			break;
		case 1:
			dist += fabs(diff);
			break;
		case 0:
			if ((adiff = fabs(diff)) > dist)
			dist = adiff;
			break;
		default:
			dist += pow(fabs(diff), (double) norm);
			break;
		}
	}

	return dist;				//compute_root(dist);
}

//ftiaksame quicksort
void quicksort(double* arr, int* indices, int left, int right) {
    if (left >= right)
        return;

    //pivot=middle element
    int pivotIndex = (left + right) / 2;
    double pivotValue = arr[pivotIndex];

    //partition array around pivot
    int i = left;
    int j = right;

    while (i <= j) {
        while (arr[i] < pivotValue)
            i++;
        while (arr[j] > pivotValue)
            j--;
        if (i <= j) {
            //swap element i with element j
            double tempValue = arr[i];
            arr[i] = arr[j];
            arr[j] = tempValue;

            //swap idences
            int tempIndex = indices[i];
            indices[i] = indices[j];
            indices[j] = tempIndex;
            i++;
            j--;
        }
    }

	//anadromiko sort twn sub-arrays
    quicksort(arr, indices, left, j);
    quicksort(arr, indices, i, right);
}

void compute_knn_brute_force(double **xdata, double *q, int npat, int lpat, int knn, int *nn_x, double *nn_d)
{
	int i, max_i;
	double max_d, new_d;

	/* initialize pairs of index and distance */
	for (i = 0; i < knn; i++) {
		nn_x[i] = -1;
		nn_d[i] = 1e99-i;
	}

	
	max_d = compute_max_pos(nn_d, knn, &max_i);
	
	#pragma omp parallel num_threads(8)
    {
        #pragma omp single nowait
        {
            #pragma omp taskloop private(i, new_d) shared(xdata, q, nn_x, nn_d, max_d) 
            for (i = 0; i < npat; i++) {

                
				new_d = compute_dist(q, xdata[i], lpat);    //euclidean
                
				if (new_d < max_d) {   	 					//add point to the list of knns, replace element max_i
                    nn_x[max_i] = i;
                    nn_d[max_i] = new_d;
                    max_d = compute_max_pos(nn_d, knn, &max_i);
                }
            }
			#pragma omp taskwait
        }
    }

	
	//valame quicksort gia sort tou knn list
    quicksort(nn_d, nn_x, 0, knn - 1);

	return;
}


/* compute an approximation based on the values of the neighbors */
double predict_value(int dim, int knn, double *xdata, double *ydata, double *point, double *dist)
{
    int i;
    double sum_v = 0.0;
    double sum_weight = 0.0;
	double weight;

    //inverse distance weighted average
    for (i = 0; i < knn; i++) {
        weight = 1.0 / dist[i]; 		//weight=inverse of distance
        sum_v += weight * ydata[i]; 	//weighted sum of function values
        sum_weight += weight; 			//sum of weights
    }

    if (sum_weight != 0.0) {
        return sum_v / sum_weight; 		//weighted average
    } else {
        return 0.0; 					//diairesi me 0 se periptwsei pou heights=0
    }
}



