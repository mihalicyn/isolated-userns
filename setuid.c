#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char **argv)
{
	if (argc < 3) {
		printf("./setuid <uid> <cmd>\n");
		exit(1);
	}

	if (setuid(atoi(argv[1])))
		perror("setuid");

	execvp(argv[2], &argv[2]);

	return 0;
}
