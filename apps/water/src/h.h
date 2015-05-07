extern double UNITT,UNITL,UNITM,BOLTZ,AVGNO,PCC[11];
/*
C.....DRIVER FOR MOLECULAR DYNAMIC SIMULATION OF FLEXIBLE WATER MOLECULE
C     WRITTEN BY GEORGE C. LIE, IBM, KINGSTON, N.Y.
C     APRIL 14, 1987 VERSION
C
C ******** INPUT TO THE PROGRAM ********
C
C   &MDINP
C
C     TEMP  : TEMPERATURE IN DEGREE K (DEFAULT=298.0).
C     RHO   : DENSITY IN G/C.C. (DEFAULT=0.998).
C     NORDER: ORDER USED TO SOLVE NEWTONIAN EQUATIONS (DEFAULT=5).
C     TSTEP : TIME STEP IN SECONDS (DEFAULT=1.0E-15).
C     NSTEP : NO. OF TIME STEPS FOR THIS RUN.
C     NPRINT: FREQUENCY OF PRINTING INTERMEDIATE DATA, SUCH AS KINETIC
C             ENERGY, POTENTIAL ENERGY, AVERAGE TEMPERATURE, ETC.
C             (ONE LINE PER PRINTING, DEFAULT=100).
C     LKT   : 1 IF RENORMALIZATION OF KINETIC ENERGY IS TO BE DONE JUST
C               BEFORE SAVING DATA FOR RESTART (NEEDED ONLY AT THE
C               BEGINNING WHERE THE ENERGY OR TEMPERATURE IS TOO HIGH);
C             0 OTHERWISE (DEFAULT=0).
C     NSAVE :-1 FOR THE VERY FIRST RUN;
C             0 DURING EQUILIBRATION STAGE;
C             N FREQUENCY OF DATA (X AND V) SAVING DURING DATA
C               COLLECTING STAGE (DEFAULT=10).
C     NRST  : FREQUENCY OF SAVING INTERMEDIATE DATA FOR RESTART.
C     CUTOFF: CUTOFF RADIUS FOR NEGLECTING FORCE AND POTENTIAL.
C             SET TO 0.0 IF HALF THE SIZE OF THE BOX IS TO BE USED
C             (DEFAULT=0.0D0)
C     NFMC  : 0 IF INITIALIZATION IS TO BE STARTED FROM REGULAR LATTICE;
C            11 IF INITIALIZATION IS TO BE STARTED FROM A RESTART FILE;
C             N IF INITIALIZATION IS TO BE STARTED FROM FT-N
C               WHICH CONTAINS THE COORDINATES OF THE WATER MOLECULES
C               IN THE FORMAT OF 5E16.8.  THE ORDERS ARE X OF H, O, H,
C               OF THE 1-ST WATER, X OF H, O, H OF THE 2-ND WATER, ....
C               FOLLOWED (STARTING FROM A NEW LINE) BY Y'S, THEN Z'S
C               (DEFAULT=12);
C            <0 TO RESET THE STATISTICAL COUNTERS.
C     NFSV  : FORTRAN FILE NO. FOR SAVED DATA (DEFAULT=10).
C     NFRST : FORTRAN FILE NO. FOR RESTART DATA (DEFAULT=11).
C
C   &END
C
*/

/* let our new files have the names associated with the fortran numbers */
extern FILE *one;
extern FILE *five;
extern FILE *six;
extern FILE *nfmc;

extern double FC11,FC12,FC13,FC33, FC111,FC333,FC112,FC113,
                FC123,FC133, FC1111,FC3333,FC1112,
                FC1122,FC1113,FC1123,FC1133,FC1233,FC1333;
#define MAXMOLS	1728

struct GlobalMemory {
        int IntrafVirLock;
        int InterfVirLock;
        int FXLock;
        int FYLock;
        int FZLock;
	int KinetiSumLock;
	int PotengSumLock;
	int MolLock[MAXMOLS];
        int start;
	int InterfBar;
	int PotengBar;
	double VIR;
	double SUM[3];
	double POTA, POTR, POTRF;
        };

extern struct GlobalMemory *gl;

typedef double vm_type[3];
typedef double mol_type[NDIR][NATOM];

typedef struct mol_dummy {
    double *VM;
    double **F[MXOD2];
} molecule_type;

extern molecule_type *VAR;



typedef struct disp_dummy {
    vm_type VM;
    double DISP[NDIR][NATOM];
} vm_disp_type;
extern vm_disp_type *VAR_VM_DISP;

typedef struct acc_dummy {
    double VEL[NDIR][NATOM];
    double ACC[NDIR][NATOM];
} vel_acc_type;
extern vel_acc_type *VAR_VEL_ACC;

typedef struct ders_dummy {
  double **DER[MAXODR-2];
} ders_type;
extern ders_type *VAR_DERS;

typedef struct ders_dummy {
  double FORCE[NDIR][NATOM];
} force_type;
extern force_type *VAR_FORCES;

extern double  TLC[100],
      ELPST,TKIN,TVIR,TTMV,FPOT,FKIN;
extern int IX[3*MXOD2+1], IRST,NVAR,NXYZ,NXV,IXF,IYF,IZF,IMY,IMZ;

#define MAXPROCS 16

extern int StartMol[MAXPROCS+1];
extern int StartPos[MAXPROCS+1];
extern int MolsPerProc;
extern double  TEMP,RHO,TSTEP,BOXL,BOXH,CUTOFF,CUT2;
extern int    NMOL,NORDER,NATMO,NATMO3,NMOL1;
#define NOMOL 343
#define MAXODR 7
#define NATOM 3
#define MXOD2 (MAXODR+2)
#define NDVAR (NATOM*NOMOL*3*MXOD2)
#define NDIR 3


extern double R3[128],R1;
extern int I2;
#define H1 0
#define O  1
#define H2 2
#define XDIR 0
#define YDIR 1
#define ZDIR 2
#define DISP 0
#define VEL 1
#define ACC 2
#define DER_3 3
#define DER_4 4
#define DER_5 5
#define DER_6 6
#define FORCES 7
extern double OMAS,HMAS,WTMOL,ROH,ANGLE,FHM,FOM,ROHI,ROHI2;
extern int NATOMS;
#define max(a,b) ( (a) < (b) ) ? b : a
#define min(a,b) ( (a) > (b) ) ? b : a
#define ONE ((double) 1)
#define TWO ((double) 2)
#define FIVE ((double) 5)
extern double  QQ,A1,B1,A2,B2,A3,B3,A4,B4,AB1,AB2,AB3,AB4,C1,C2,
               QQ2,QQ4,REF1,REF2,REF4;
