#include <stdio.h>
#include <math.h>
#include <jia.h>

double f(double a)
{
    return (4.0 / (1.0 + a*a));
}

int main(int argc,char *argv[])
{
    int n,i,begin,end;
    double PI25DT = 3.141592653589793238462643;
    double mypi, h, sum, x, a;
    float startt, endt;
    double *pa;

   
    jia_init(argc,argv); 
    n = 1000000;
    pa=(double *)jia_alloc(sizeof(double));
    jia_barrier();
    
    if (jiapid==0) {
      *pa =0.0;
    }
    jia_barrier();
    startt = jia_clock();

    h   = 1.0 / (double) n;
    sum = 0.0;
    begin = n/jiahosts*jiapid+1;
    end = n/jiahosts*(jiapid+1);
  
    for (i = begin; i <= end; i++){
      x = h * ((double)i - 0.5);
      sum += f(x);
    }
    mypi = h * sum;

    jia_lock(1);
     *pa= *pa+mypi;
    jia_unlock(1);
    jia_barrier();
    endt = jia_clock();

    if (jiapid==0) {
       printf("pi is approximately %.16f, Error is %.16f\n",
       *pa, fabs(*pa - PI25DT));
       printf("Elapsed time = %f\n", endt-startt);
    }
}

            
