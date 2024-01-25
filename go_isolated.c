#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("%s <cmd> [<arg1> ...]\n", argv[0]);
		exit(1);
	}

	if (unshare(CLONE_NEWUSER)) {
		perror("unshare");
		exit(1);
	}

	int fd_isol = open("/proc/self/isolated_uns", O_WRONLY);
	if (fd_isol < 0) {
		perror("open");
		exit(1);
	}

	if (write(fd_isol, "yes", 3) < 0) {
		perror("write");
		close(fd_isol);
		exit(1);
	}

	close(fd_isol);

	/*
	 * At this point our current_euid() looks like (0, some_uid).
	 * But this kuid_t value has no mapping to current_user_ns(),
	 * because we have not installed the UID/GID mappings and we don't want to!
	 * But this means that even inside the user namespace our UID == overflowuid.
	 *
	 * Another problem is that just after we do exec*() syscall we'll lose all the capabilities!
	 * The only exclusion from this rule is a "root user" in another words:
	 * root_uid = make_kuid(new->user_ns, 0);
	 * uid_eq(cred->euid, root_uid) must be true.
	 * (Please refer to handle_privileged_root() in the kernel for details.)
	 *
	 * At this point make_kuid(current_user_ns(), 0) value is a VALID (while unmapped to the host!)
	 * and looks like (some_user_namespace_id, 0). At the same time cred->euid value looks like
	 * (0, some_uid). Obviously, uid_eq(cred->euid, root_uid) is not true.
	 *
	 * Let's do setuid/setgid to 0 and change cred->euid to be equal to make_kuid(new->user_ns, 0).
	 * This will make next execvp(..) call not to drop capabilites from us.
	 *
	 */
	if (setuid(0) || setgid(0)) {
		perror("setuid/setgid");
		exit(1);
	}

	execvp(argv[1], &argv[1]);

	return 0;
}
