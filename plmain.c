#include	<stdio.h>

#include	"printlog.h"

main(int argc, char *argv[])
{
	if (argc< 3) {
		fprintf(stderr, "specify input, output filenames\n") ;
		exit(1) ;
	}
	printlog(argv[1], argv[2]) ;
}

