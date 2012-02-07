/* findminmax_seq.c - find the min and max values in a random array
 *
 * usage: ./findminmax_seq <number_file> <count>
 *
 */
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<pthread.h>
#include"measure.h"

/* a struct used to pass results to caller */
struct results {
    double min;
    double max;
};

/*global variable*/
 struct results r;
 int nthreads;
 double *array;
 int arraysize = 0;
 pthread_mutex_t gmutex = PTHREAD_MUTEX_INITIALIZER;
 
/* given an array of ints and the size of array, find min and max values */
struct results find_min_and_max(double *subarray, int offset, int chunksize)
{
    int i;
    double min, max;
    min = max = subarray[0];
    struct results rtemp;
    
    for (i = offset; i < (offset + chunksize); i++) {
        if (subarray[i] < min) {
            min = subarray[i];
        }
        if (subarray[i] > max) {
            max = subarray[i];
        }
    }
    rtemp.min = min;
    rtemp.max = max;
    return rtemp;
}

/* read a file containing a list of doubles */
void read_file_doubles(char *filename, double a[], int count)
{
    FILE *f;
    double fval;
    int i = 0;
    
    f = fopen(filename, "r");
    assert(f != NULL);
    
    while (fscanf(f, "%lf\n", &fval) != EOF) {
        if (i >= count) {
            break;
        }
        a[i] = fval;
        i++;
    }
    
    if (i != count) {
        printf("read_file_doubles(): only read %d doubles out of %d\n",
            i, count);
        exit(1);
    }
}

void *find_thread(void *arg)
{
	struct results rtemp;
    int index = (int) arg;
    int chunksize = 0;
    int offset = 0;

    /* compute count for substring */
    chunksize = arraysize / nthreads;
    offset = index * chunksize;

    rtemp = find_min_and_max(array, offset, chunksize);

 	pthread_mutex_lock(&gmutex);
    if (r.min > rtemp.min)
    	r.min = rtemp.min;
    if (r.max < rtemp.max)
    	r.max = rtemp.max;
    pthread_mutex_unlock(&gmutex);
}

int main(int argc, char **argv)
{
    pthread_t *pt_array;
    int *rv_array;
    int rv;
   	int i;

    /* process command line arguments */
    if (argc != 4) {
        printf("usage: ./findminmax_seq <filename> <count> <nthreads>\n");
        return 1;
    }

    arraysize = atoi(argv[2]);
    nthreads = atoi(argv[3]);
    assert (nthreads > 0);
    
    pt_array = (pthread_t *) malloc(sizeof(pthread_t) * nthreads);
    rv_array = (int *) malloc(sizeof(int) * nthreads);

    /* allocate array and populate with random values */
    array = (double *) malloc(sizeof(double) * arraysize);
    
    
    read_file_doubles(argv[1], array, arraysize);
   	r.min = array[0];
    r.max = array[0];
    
   /* begin computation */
    measure_begin();

    for (i = 0; i < nthreads; i++) {
        rv = pthread_create(&pt_array[i], NULL, find_thread, (void *) i);
        assert(rv == 0);
    }

    for (i = 0; i < nthreads; i++) {
        pthread_join(pt_array[i], (void **) &rv_array[i]);
    }
    
    measure_end();
    
    printf("Execution time: ");
    measure_print_seconds(1);

    printf("min = %lf, max = %lf\n", r.min, r.max);

    return 0;
}
