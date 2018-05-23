	#include <stdio.h>
	#include <stdlib.h>
	#include <pthread.h>

	#define MESSAGES 20
	#define NThreads 4
	#define N 1000
	#define CUTOFF 1000

	struct message{																											// exei mesa ta stoixia tou pinaka							
		int start;
		int finish;
	};
	typedef struct message message;																									// define variables "message"


	message globalBuffer[MESSAGES];																							// global integer buffer	-	kyklikos buffer
	int global_availmsg = 0;																								// empty , // global avail messages count (0 or 1)
																					
	pthread_cond_t msg_in = PTHREAD_COND_INITIALIZER;																		// gia na parei entolh gia to pote mporei na diavasei apo ton buffer mono ena thread
																															// condition variable, signals a put operation (receiver waits on this)
	pthread_cond_t msg_out = PTHREAD_COND_INITIALIZER;									 									// gia na stilei entolh gia to pote mporei na grapsei ston buffer mono ena thread
																															// condition variable, signals a get operation (sender waits on this)					
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;   							 										//to komati kwdika pou trexei exei mono auto prosvash ston kwdika  
																															// mutex protecting common resources


	void inssort(double *a,int n) {							 																// xrisimipoioume insort logo tou oti to orio (cutoff) einai megalutero apo to mege8os tou pinaka
	int i,j;																												// apo to paradigma seiriakhs uloipoihshs (append komatiwn pinaka se ena)			
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
	  if (n<=CUTOFF) {																										//otan ena komati pinaka exei mikrinei polu tote den kanoume quicksort alla inssort
		inssort(a, n);
		return 0;
	  }
	  
																															// put median-of-3 in the middle
	  if (a[middle]<a[first]) { t = a[middle]; a[middle] = a[first]; a[first] = t; }										//sigkrish tou prwtou me to mesaio koutaki (an isxuei h sin8ikh kanoume antimeta8esh) k.o.k.
	  if (a[last]<a[middle]) { t = a[last]; a[last] = a[middle]; a[middle] = t; }											//sigkrish tou mesaiou me to teleuteo koutaki			
	  if (a[middle]<a[first]) { t = a[middle]; a[middle] = a[first]; a[first] = t; }
		
																															// partition (first and last are already in correct half)
	  p = a[middle]; 																										// pivot
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

																															// producer thread function
	void *producer_thread(void *args) {
	  int i;
	  message Message;
	  
																															// send a predefined number of messages
	  for (i=0;i<MESSAGES;i++) {
																															// lock mutex (kleidwnei 
		pthread_mutex_lock(&mutex);
		while (global_availmsg>0) {																							// NOTE: we use while instead of if! more than one thread may wake up
						
		  pthread_cond_wait(&msg_out,&mutex);  																				// wait until a msg is received - NOTE: mutex MUST be locked here.
																															// If thread is going to wait, mutex is unlocked automatically.
																															// When we wake up, mutex will be locked by us again. 
		}
																															// send message
		globalBuffer[i].start = i;
		printf("Producer: sending msg %d\n",globalBuffer[i].start);
		global_availmsg = 1;
		
																															// signal the receiver that something was put in buffer
		pthread_cond_signal(&msg_in);
		
																															// unlock mutex
		pthread_mutex_unlock(&mutex);
	  }
	  
																															// exit and let be joined
	  pthread_exit(NULL); 
	}
	  
	  
																															// receiver thread function
	void *consumer_thread(void *args) {
	  int i;
	  
																															// receive a predefined number of messages
	  for (i=0;i<MESSAGES;i++) {
																															// lock mutex
		pthread_mutex_lock(&mutex);
		while (global_availmsg<1) {																							// NOTE: we use while instead of if! more than one thread may wake up
		  pthread_cond_wait(&msg_in,&mutex); 
		}
																															// receive message
		printf("Consumer: received msg %d\n",globalBuffer[i]);
		global_availmsg = 0;
		
																															// signal the sender that something was removed from buffer
		pthread_cond_signal(&msg_out);
		
																															// unlock mutex
		pthread_mutex_unlock(&mutex);
	  }
	  
																															// exit and let be joined
	  pthread_exit(NULL); 
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
	  
																															//Creating the threadpool
	  pthread_t threads[NThreads];																								
	  int t;	
	  for (t=0; t< NThreads; t++){
		pthread_create(&(threads[t]), NULL, producer_thread, NULL);															//dimiourgia thread (to prwto orisma einai to pou 8a apo8ikeutei to id tou kainourio thread), to producer_thread einai h main function gia to thread (3ekinaei apo edw)
	 }
	  
																															//error check
	  
		for(i = 0; i < (N -1); i++) {
			printf ("a[%d] : %d \n", i, a[i]);
		//	if(a[i] > a[i + 1]) {
		//		printf("Error at element %d\n", i);
		//		break;
		//	}
		}
	  pthread_t producer,consumer;
	  
	  pthread_mutex_destroy(&mutex);
	  pthread_cond_destroy(&msg_out);
	  pthread_cond_destroy(&msg_in);

	  return 0;
	}