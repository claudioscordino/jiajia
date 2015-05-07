#include <jia.h>
 
#define N  1024
float (*a)[N], (*b)[N], (*c)[N];

void seqinit()
{int i,j;

   if (jiapid==0) {
     for (i=0;i<N;i++) 
       for (j=0;j<N;j++){
         a[i][j]=1.0;
         b[i][j]=1.0;
     }
   }
}

void worker()
{int i,j,k;
 int start, end;
 float temp;

 start=(N/jiahosts)*jiapid;
 end=start+(N/jiahosts);

 for (j=0;j<N;j++){ 
   for (i=start;i<end;i++){
       temp=0.0;
       for (k=0;k<N;k++)
         temp+=a[i][k]*b[j][k];
       c[i][j]=temp;
       if (i==start)
         printf("c[%d][%d]=%f\n",i,j,c[i][j]);
   }
 }
}


main(int argc,char **argv)
{int i,j;
 float t1,t2;

 jia_init(argc,argv);
 
 a=(float (*)[N])jia_alloc3(N*N*sizeof(float),(N*N*sizeof(float))/jiahosts,0);
 b=(float (*)[N])jia_alloc3(N*N*sizeof(float),(N*N*sizeof(float))/jiahosts,0);
 c=(float (*)[N])jia_alloc3(N*N*sizeof(float),(N*N*sizeof(float))/jiahosts,0);

 jia_barrier();

 seqinit();
 
 jia_barrier();
 jia_startstat();
 t1=jia_clock();

 worker();

 jia_barrier();
 t2=jia_clock();
 if (jiapid==0) 
   printf("Total time for matric multiply is == %10.2f seconds\n", t2-t1);

 jia_exit();
} 
 


