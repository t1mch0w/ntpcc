#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>  /* Many POSIX functions (but not all, by a large margin) */
#include <fcntl.h>   /* open(), creat() - and fcntl() */


#include "tpc.h"      /* prototypes for misc. functions */
#include "ndriver.h" 

#define NUM_TXN 5
#define THINKING_TIME 10
#define KEYING_TIME 10
#define num_ware 10

int other_ware (int home_ware);
void send_request(int fd) ;
void prepare_neworder() ;

extern struct TXNLIST *txnlist;

struct TXN *txn;

int  notfound = MAXITEMS+1;
int fd;

//Driver's thread
void ndriver(int w_id, int d_id) {
	int n_txn;
	clock_t clk1,clk2;
	double rt;
	struct timespec tbuf1;
	struct timespec tbuf2;

	unsigned int seed;

	seed = time(NULL);

	while(1) {
		sleep(1);
		txn = malloc(sizeof(struct TXN));

    clk1 = clock_gettime(CLOCK_REALTIME, &tbuf1 );
		//Sleep for keying time
		//sleep(KEYING_TIME);
		n_txn=rand_r(&seed)%NUM_TXN;

		txn->txn_type=n_txn;

		printf("Before send, w_id=%d\n",txn->txn_data.neworder.w_id);
		switch (n_txn) {
			case 0:
				prepare_neworder(&txn->txn_data.neworder);
				break;
			default:
				prepare_neworder(&txn->txn_data.neworder);
		}
		send_request(fd);
		printf("After send, w_id=%d\n",txn->txn_data.neworder.w_id);
    clk2 = clock_gettime(CLOCK_REALTIME, &tbuf2 );
		//Sleep for thinking time
		//sleep(THINKING_TIME);
		rt = (double)(tbuf2.tv_sec + tbuf2.tv_nsec/1000000000.0-tbuf1.tv_sec - tbuf1.tv_nsec/1000000000.0);
		free(txn);
	}
}

void prepare_neworder(struct NEWORDER *neworder) {
	int rbk;
  int i;
  
	neworder->w_id = RandomNumber(1, num_ware);
	//printf("w_id=%d",neworder->w_id);
	neworder->d_id = RandomNumber(1, DIST_PER_WARE);
	neworder->c_id = NURand(1023, 1, CUST_PER_DIST);
	neworder->ol_cnt = RandomNumber(5, 15);
	rbk = RandomNumber(1, 100);
	
	for (i=0;i<neworder->ol_cnt;i++) {
		neworder->itemid[i] = NURand(8191, 1, MAXITEMS);
		if ((i == neworder->ol_cnt - 1) && (rbk == 1)) {
			neworder->itemid[i] = notfound;
		}
		if (RandomNumber(1, 100) != 1) {
			neworder->supware[i] = neworder->w_id;
		}
		else {
			neworder->supware[i] = other_ware(neworder->w_id);
			neworder->all_local = 0;
		}
		neworder->qty[i] = RandomNumber(1, 10);
	}
}

int other_ware (int home_ware)
{
    int tmp;
    if (num_ware == 1) return home_ware;
    while ((tmp = RandomNumber(1, num_ware)) == home_ware);
    return tmp;
}

void send_request(int fd) {
	int n;
	char buffer[256];
	
	//printf("txn size=%d\n", sizeof(struct TXN));
	//printf("txn w_id=%d\n", txn->txn_data.neworder.w_id);
	//n = send(fd, (void *) txn, sizeof(struct TXN), 0);
	n = write(fd, txn, sizeof(struct TXN));

	//buffer[0]='a';
	//n = write(fd, buffer, 1);


	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}

	/* Now read server response */
	bzero(buffer,256);
	n = read(fd, buffer, 255);

	if (n < 0) {
		perror("ERROR reading from socket");
		exit(1);
	}

	printf("%s\n",buffer);

}

int main(int argc, char *argv[]) {
	int portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[256];

	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}

	portno = atoi(argv[2]);

	/* Create a socket point */
	fd = socket(AF_INET, SOCK_STREAM, 0);

	if (fd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}

	server = gethostbyname(argv[1]);

	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	/* Now connect to the server */
	if (connect(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR connecting");
		exit(1);
	}

	ndriver(0,0);

	return 0;
}
