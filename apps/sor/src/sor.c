/*
 * A Red-Black SOR
 *
 *	using separate red and block matrices
 *	to minimize false sharing
 *
 * Solves a M+2 by 2N+2 array
 */
#include <stdio.h>
#include <sys/time.h>
#include <jia.h>

/*
 * To begin statistics collection after the first iteration
 * has completed use the following #define
 *
#define	RESET_AFTER_ONE_ITERATION
 */
int	iterations = 100;

int	M = 2046;
int	N = 1023;	/* N.B. There are 2N columns. */

float **red_;
float **black_;
float t1,t2;

print_board()
{int i,j;

 for (i=0;i<=N;i+=0x40){
   printf("red[%4d][%4d]= %10.3f): ", i,i,red_[i][i]);
   printf("----- black[%4d][%4d]= %10.3f): ", i,i,black_[i][i]);
   printf("\n");
 }
}


void sor(begin, end)
int	begin;
int	end;
{int i,j,k;
   
 for (i=0;i<iterations;i++){
   printf("iteration %d\n",i);
   if (i==1) t1=jia_clock();
                
   if (begin&1) { /*begin is odd*/
     for (j = begin; j <= end; j++) {
       for (k = 0; k < N; k++) {
         black_[j][k] = (red_[j-1][k]+red_[j+1][k]+red_[j][k]+red_[j][k+1])*0.25;
       }
       if ((j += 1) > end) break;
       for (k = 1; k <= N; k++) {
         black_[j][k] = (red_[j-1][k]+red_[j+1][k]+red_[j][k-1]+red_[j][k])*0.25;
       }
     }
     jia_barrier();
     for (j = begin; j <= end; j++) {
       for (k = 1; k <= N; k++) {
         red_[j][k] = (black_[j-1][k]+black_[j+1][k]+black_[j][k-1]+black_[j][k])*0.25;
       }
       if ((j += 1) > end) break;
       for (k = 0; k < N; k++) {
         red_[j][k] = (black_[j-1][k]+black_[j+1][k]+black_[j][k]+black_[j][k+1])*0.25;
       }
     }
     jia_barrier();
   }else{  /*begin is even*/
     for (j = begin; j <= end; j++) {
       for (k = 1; k <= N; k++) {
         black_[j][k] = (red_[j-1][k]+red_[j+1][k]+red_[j][k-1]+red_[j][k])*0.25;
       }
       if ((j += 1) > end) break;
       for (k = 0; k < N; k++) {
         black_[j][k] = (red_[j-1][k]+red_[j+1][k]+red_[j][k]+red_[j][k+1])*0.25;
       }
     }
     jia_barrier();
     for (j = begin; j <= end; j++) {
       for (k = 0; k < N; k++) {
         red_[j][k] = (black_[j-1][k]+black_[j+1][k]+black_[j][k]+black_[j][k+1])*0.25;
       }
       if ((j += 1) > end) break;
       for (k = 1; k <= N; k++) {
         red_[j][k] = (black_[j-1][k]+black_[j+1][k]+black_[j][k-1]+black_[j][k])*0.25;
       }
     }
     jia_barrier();
   }
 }
}

extern char *optarg;

slave()
{int begin,end;
	
 begin = ((M+2)*jiapid)/jiahosts;
 end   = ((M+2)*(jiapid+1))/jiahosts-1;
 if (jiapid==0) begin++;
 if (jiapid==(jiahosts-1)) end--;

 jia_startstat();
 t1=jia_clock();

 sor(begin, end);

 jia_barrier();
 t2=jia_clock();
 if (jiapid==0){
   printf("Elapsed time: %.2f seconds\n",t2-t1);
 }
}


main(argc, argv)
int		argc;
char	       *argv[];
{int c,i,j;
 float sum;

 while ((c = getopt(argc, argv, "i:m:n:")) != -1)
 switch (c) {
   case 'i':
     iterations = atoi(optarg);
     break;
   case 'm':
     M = atoi(optarg);
     break;
   case 'n':
     N = atoi(optarg);
     break;
   }

 jia_init(argc, argv);

 red_ = (float **) malloc((M + 2)*sizeof(float *));
 black_ = (float **) malloc((M + 2)*sizeof(float *));
	
 for (i = 0; i <= M + 1; i++) {
   red_[i]=(float*)jia_alloc2p((N+1)*sizeof(float),i/((M+2)/jiahosts)); 
   black_[i]=(float*)jia_alloc2p((N+1)*sizeof(float),i/((M+2)/jiahosts));
/*
   red_[i] = (float *) jia_alloc((N+1)*sizeof(float));
   black_[i] = (float *) jia_alloc((N+1)*sizeof(float));
*/
 }

 jia_barrier();	

 if (jiapid==0) {
   for (i = 0; i <= M + 1; i++) {
/* 
   for (i = (M+2)*jiapid/jiahosts; i < (M+2)*(jiapid+1)/jiahosts; i++) {
*/
     for (j = 0, sum = 0.0; j <= N; j++) {
       red_[i][j] = black_[i][j] = sum += (float)j;
     }

     if ((i==0)||(i==M+1)){
       for (j = 0; j <= N; j++)
	 red_[i][j] = black_[i][j] = 1.0;
     }else if (i&1) {
       red_[i][0] = 1.0;
       black_[i][N] = 1.0;
     }else{
       black_[i][0] = 1.0;
       red_[i][N] = 1.0;
     }
   }
 }

        jia_barrier();

/*
 jia_config(HMIG,ON);
*/	
 jia_config(WVEC,ON);
 jia_config(ADWD,ON);

 slave();
 jia_exit();
}


