#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#define nsems 1
#define P_OPERATION -1
#define V_OPERATION 1

void main (int argc, char *argv []) {
	int fd, start, size, n_acc, acc_no, amount, semid;
	key_t key = getuid ();
	typedef struct {
		char name [20];
		char phone_no [11];
		unsigned int balance;
	}RECORD;
	RECORD record;
	RECORD *ptr = &record;
	struct sembuf {
		unsigned short sem_num;
		short sem_op;
		short sem_flg;
	};
	struct sembuf oper = {0, P_OPERATION, SEM_UNDO};
	struct sembuf *sops = &oper;

	if (argc != 3 ) {
		fprintf (stderr, "To deposit use: deposit <acc_no> <amount>\n");
		exit (1);
	}
	if ((semid = semget (key, nsems, 0600)) == -1) {
		perror ("Can't associate semaphore facility");
		exit (2);
	}
	fd = open ("sb_acc", O_RDWR);
	start = lseek (fd, 0, SEEK_CUR);
	size = lseek (fd, 0, SEEK_END);
	n_acc = size / sizeof (record);
	lseek (fd, start, SEEK_SET);
	sscanf (argv [1], "%d", &acc_no);
	if (acc_no <= 0 || acc_no > n_acc) {
		fprintf (stderr, "Account no invalid\n");
		exit (3);
	}
	sscanf (argv [2], "%d", &amount);
	if (amount <= 0) {
		fprintf (stderr, "Cash deposit not possible\n");
		exit (4);
	}
	if (semop (semid, sops, 1) == -1) {
		perror ("Sem operation failed\n");
		exit (5);
	}
	lseek (fd, (acc_no - 1) * sizeof (record), SEEK_CUR);
	if (read (fd, ptr, sizeof (record)) == -1) {
		perror ("read account failed");
		exit (6);
	}
	ptr -> balance += amount;
	lseek (fd, start, SEEK_SET);
	lseek (fd, (acc_no - 1) * sizeof (record), SEEK_CUR);
	if (write (fd, ptr, sizeof (record)) == -1) {
		perror ("Cash deposit failed");
		exit (7);
	}
	lseek (fd, start, SEEK_SET);
	sops -> sem_op = V_OPERATION;
	if (semop (semid, sops, 1) == -1) {
		perror ("Sem operation failed");
		exit (8);
	}
	close (fd);
	printf ("Name: %s\n", ptr -> name);
	printf ("Phone: %s\n", ptr -> phone_no);
	printf ("Account balance = %u\n", ptr -> balance);
	exit (0);
}
