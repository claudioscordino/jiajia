#define MAXMOLS	1728

struct GlobalMemory {
        int IntrafVirLock;
        int InterfVirLock;
        int FXLock;
        int FYLock;
        int FZLock;
        int KinetiSumLock;
        int PotengSumLock;
        int MolLock[MAXPROCS];
        int start;
        int InterfBar;
        int PotengBar;
	double VIR;
	double SUM[3];
	double POTA, POTR, POTRF;
        };

extern struct GlobalMemory *gl;

