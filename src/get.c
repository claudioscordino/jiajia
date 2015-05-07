extern jia_pid, hostc;

struct jia_ident {
	int jiapid;
	int jiahosts;
} jia_;

void ident_()
{
	jia_.jiapid = jia_pid;
	jia_.jiahosts = hostc;
	return;
}

/*
int getpid_()
{
	return(jia_pid);
}

int gethosts_()
{
	return(hostc);
}
*/
