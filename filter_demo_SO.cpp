#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

#define N 512
#define M 512
/*
para compilar:

sudo g++ -I/usr/local/include/opencv -I/usr/local/include/opencv2 filter_demo_SO.cpp -o ejemplo_filtro -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs

para ejecutar:
./ejemplo_filtro lena.jpg

para medir el tiempo:
 
time ./ejemplo_filtro lena.jpg

*/

using namespace cv;
/// Declare variables
Mat src, dst, dstx, dsty;
Mat kernel;
Point anchor;
double delta;
int ddepth;
int kernel_size;
int nthreads;

void *CalculoporHilo(void *);

/** @function main */
int main ( int argc, char** argv )
{ 
  ///Configurando afinidad de CPU
  long ncores = sysconf(_SC_NPROCESSORS_CONF);
  nthreads=(int)ncores;
  if(nthreads==6){
     nthreads=4;
  }
  cpu_set_t *setp = CPU_ALLOC(nthreads);
  ulong setsz = CPU_ALLOC_SIZE(nthreads);
    

  //hilos del sistema
  pthread_t tida[nthreads];
  int thread_args[nthreads];
  pthread_attr_t attr;

  /* get the default attributes */
  pthread_attr_init(&attr);

  /* set the scheduling algorithm to PCS or SCS */
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  //printf("Process id %d\n",getpid());
  //pthread_attr_setschedpolicy(&attr, SCHED_RR);
  setpriority(PRIO_PROCESS, getpid(), -20);
  

  /// Load an image
  src = imread( argv[1] );
  dst=src;//cambiar
  if( !src.data )
  { return -1; }
  /// Initialize arguments for the filter
  anchor = Point( -1, -1 );
  delta = 0;
  ddepth = -1;

  //printf("Matriz tamano %d,%d",src.rows,src.cols);
  //Gaussian
  kernel=getGaussianKernel(15,3);
  //filter2D(src, dst, ddepth , kernel, anchor, delta, BORDER_DEFAULT );
  for(int i=0; i < nthreads; i++)
  {
	CPU_ZERO_S(setsz, setp);  
	CPU_SET(i, setp);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), setp);
        thread_args[i] = i;
        pthread_create( &tida[i], NULL, CalculoporHilo, &thread_args[i]);
  }

  for(int j=0; j < nthreads; j++)
  {
        pthread_join( tida[j], NULL);
  }

  bool isSuccess = imwrite("output_lena.jpg",dst); 
  CPU_FREE(setp);
  return 0;
}

void *CalculoporHilo(void *args)
{
    int *threadindex = (int *)args;
    
    for( int i = 0; i < N; i=i+N/nthreads) {
        if (i % nthreads != *threadindex) continue;
        for( int j = 0; j < M; j=j+M/nthreads) {
            filter2D(src(Rect(i, j,N/nthreads,M/nthreads)), dst(Rect(i, j,N/nthreads,M/nthreads)), ddepth , kernel, anchor, delta, BORDER_DEFAULT );
        }
    }

    
}

