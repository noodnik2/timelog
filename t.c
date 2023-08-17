#define		LPSTR		char __far *
#define		NULL		0

static void dump(void *p, int size)
{
	int i ;
	unsigned char *pcview ;

	pcview= (unsigned char *) p ;
	for (i= 0; i< size; i++) {
		printf("0x%02X ", pcview[i]) ;
	}
	printf("\n") ;
}

main()
{
	char *psz ;
	LPSTR lpsz ;

	psz= (char *) NULL ;
	lpsz= (LPSTR) psz ;

	dump((void *) &lpsz, sizeof(lpsz)) ;
}
