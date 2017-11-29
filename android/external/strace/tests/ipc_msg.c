#include <stdio.h>
#include <errno.h>
#include <sys/msg.h>

int
main(void)
{
	int rc, id;
	struct msqid_ds ds;

	id = msgget(IPC_PRIVATE, 0600);
	if (id < 0)
		return 77;
	printf("msgget\\(IPC_PRIVATE, 0600\\) += %d\n", id);

	if (msgctl(id, IPC_STAT, &ds))
		goto fail;
	printf("msgctl\\(%d, (IPC_64\\|)?IPC_STAT, %p\\) += 0\n", id, &ds);

	int max = msgctl(0, MSG_INFO, &ds);
	if (max < 0)
		goto fail;
	printf("msgctl\\(0, (IPC_64\\|)?MSG_INFO, %p\\) += %d\n", &ds, max);

	rc = msgctl(id, MSG_STAT, &ds);
	if (rc != id) {
		/*
		 * In linux < v2.6.24-rc1 the first argument must be
		 * an index in the kernel's internal array.
		 */
		if (-1 != rc || EINVAL != errno)
			goto fail;
		printf("msgctl\\(%d, (IPC_64\\|)?MSG_STAT, %p\\) += -1 EINVAL \\(Invalid argument\\)\n", id, &ds);
	} else {
		printf("msgctl\\(%d, (IPC_64\\|)?MSG_STAT, %p\\) += %d\n", id, &ds, id);
	}

	rc = 0;
done:
	if (msgctl(id, IPC_RMID, 0) < 0)
		return 1;
	printf("msgctl\\(%d, (IPC_64\\|)?IPC_RMID, 0\\) += 0\n", id);
	return rc;

fail:
	rc = 1;
	goto done;
}
