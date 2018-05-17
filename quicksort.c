#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MESSAGES 20
#define N 10000
#define CUTOFF 1000

struct message{
	int start;
	int finish;
};

typedef struct message message;

message globalBuffer[MESSAGES];																	// global integer buffer	
int global_availmsg = 0;																		// empty , // global avail messages count (0 or 1)
																				
pthread_cond_t msg_in = PTHREAD_COND_INITIALIZER;												// gia na parei entolh gia to pote mporei na diavasei apo ton buffer mono ena thread
																								// condition variable, signals a put operation (receiver waits on this)
pthread_cond_t msg_out = PTHREAD_COND_INITIALIZER;									 			// gia na stilei entolh gia to pote mporei na grapsei ston buffer mono ena thread
																								// condition variable, signals a get operation (sender waits on this)					
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;   							 				//to komati kwdika pou trexei exei mono auto prosvash ston kwdika  
																								// mutex protecting common resources


void inssort(double *a,int n) {
int i,j;
double t;
  
  for (i=1;i<n;i++) {
    j = i;
    while ((j>0) && (a[j-1]>a[j])) {
      t = a[j-1];  
	  a[j-1] = a[j];  
	  a[j] = t;
      j--;
    }
  }

}


void quicksort(double *a,int n) {
	
	double t,p;
	int i,j;
	int first = 0;
	int middle = n/2;
	int last = n-1;  

  // check if below cutoff limit
  if (n<=CUTOFF) {
	inssort(a, n);
    return 0;
  }
  
  // put median-of-3 in the middle
  if (a[middle]<a[first]) { t = a[middle]; a[middle] = a[first]; a[first] = t; }
  if (a[last]<a[middle]) { t = a[last]; a[last] = a[middle]; a[middle] = t; }
  if (a[middle]<a[first]) { t = a[middle]; a[middle] = a[first]; a[first] = t; }
    
  // partition (first and last are already in correct half)
  p = a[middle]; // pivot
  for (i=1,j=n-2;;i++,j--) {
    while (a[i]<p) i++;
    while (p<a[j]) j--;
    if (i>=j) break;

    t = a[i]; a[i] = a[j]; a[j] = t;      
  }
   
  // recursively sort halves
  quicksort(a,i);
  quicksort(a+i,n-i);
  
}



int main() {
  double *a;
  int i;
  
  //mallocing array
  a = (double *)malloc(N*sizeof(double));
  if (a==NULL) {
    printf("error in malloc\n");
    exit(1);
  }


  // fill array with random numbers
  srand(time(NULL));
  for (i=0;i<N;i++) {
    a[i] = (double)rand()/RAND_MAX;
  }

  // error check
	for(i = 0; i < (N -1); i++) {
		printf ("a[%d] : %d \n", i, a[i]);
		if(a[i] > a[i + 1]) {
			printf("Error at element %d\n", i);
			break;
		}
	}
  
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&msg_out);
  pthread_cond_destroy(&msg_in);

  return 0;
}