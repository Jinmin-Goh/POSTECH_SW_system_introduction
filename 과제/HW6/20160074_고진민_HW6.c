// 20160074 화학공학과 고진민
// HW6

// 1. (9.14)
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
	int fd;		// file descriptor
	struct stat stat;
	char *buf;
	if((fd = open("hello.txt", O_RDWR, NULL)) < 0)	// open the file
		exit(1);	// error
	if(fstat(fd, &stat))	// retrive information of fd
		exit(1);	// error
	// buf is pointer to mapped area
	if((buf = (char*)mmap(NULL, stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fsd, 0)) < 0)
		exit(1);	// error

	buf[0] = 'J';	// changing first letter to J

	if(munmap(buf, stat.st_size))	// delete mapped memory
		exit(1);	// error
	if(close(fd))	// close file
		exit(1);	// error
	return 0;
}

// 4. (10.8)
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	struct stat stat;
	char *type, *readok;
	int fd;

	fd = atoi(argv[1]);		// taking descrpitor number from command line
	fstat(fd, &stat);		// retriving information of fd

	// remaining part is identical to Figure 10.10
	if(S_ISREG(stat.st_mode))
		type = "regular";
	else if(S_ISDIR(stat.st_mode))
		type = "directory";
	else
		type = "other";

	if((stat.st_mode & S_IRUSR))
		readok = "yes";
	else
		readok = "no";

	printf("type: %s, read: %s\n", type, readok);
	exit(0);
}
