typedef double vm_type[3];
typedef double d_type[NDIR][NATOM];

typedef struct mol_dummy {
    double *VM;
    double *F[MXOD2][NDIR];
} molecule_type;
extern molecule_type *VAR;


typedef struct shared_dummy {
  vm_type Vm;
  double Disp[NDIR][NATOM];
} shared_type;
extern shared_type *VAR_DISPVM;


typedef struct sh_dummy {
  double Force[NDIR][NATOM];
} force_type;
extern force_type *VAR_FORCE;

typedef struct private_dummy {
    double Der[MAXODR-1][NDIR][NATOM];
} private_type;
extern private_type *VAR_PRIVATE;

extern double  TLC[100],
      ELPST,TKIN,TVIR,TTMV,FPOT,FKIN;
extern int IX[3*MXOD2+1], IRST,NVAR,NXYZ,NXV,IXF,IYF,IZF,IMY,IMZ;

#define MAXPROCS 128

extern int StartMol[MAXPROCS+1];
extern int StartPos[MAXPROCS+1];
extern int MolsPerProc;
