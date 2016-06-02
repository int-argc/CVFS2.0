// Decommissioning of targets
// take note: I am using the shared db variable in global_defs
// note again: this version assumes bytes yung naka store sa db.
// pero... sa tingin ko kelangan natin ata i kilobytes..?
// in that case papalitan ko lang naman yung toBytes na function?
// actually takot lang ako na baka di kasya pag punta sa program
// pero medyo nakakagulat nga na kasya pala sa long!!!! hmm pero ewan ko pa rin

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

#include "global_defs.h"
#include "cmd_exec.h"

static int callback(void *used, int argc, char **argv, char **colname) {
	int i;
	string comm = "";
	long occsp = (long)used;
	if(argc != 1) {
		fprintf(stderr, "Error in database table: missing columns\n");
		printf("FAILED");
		exit(1);
	} else {
		// compare if the available space in other targets can accomodate
		// the occupied space in the leaving node
		long total_av = atol(argv[0]);
		printf("total available space = %ld\n", total_av);
		if (total_av >= occsp) {
			printf("you can decommission now!\n");

		} else {
			printf("You cannot decommission, not enough space.\n");
			printf("FAILED");
			exit(0);
		}
	}

	return 0;
}

int main(int argc, char *argv[]) {
    char *errmsg = 0;
    int rc;
	long occsp;

	if(argc != 3){
		printf("FATAL: Program takes exactly 2 arguments.\n");
		printf("Usage:\n");
		printf("\tdecomm <IQN> <mount point>\n");	// this is to limit db queries
		printf("FAILED");
		exit(1);
	}

	// at this point, we assume that the IQN is valid
	string iqn = "", query = "", comm = "", comm_out = "";
	strcpy(iqn, argv[1]);
	printf("IQN = %s\n", iqn);
	//sizex = <occupied space in x>
	sprintf(comm, "du -s %s | awk '{print $1}'", argv[2]);
	runCommand(comm, comm_out);
	occsp = atol(comm_out) * 1000;	// convert to bytes
	printf("occupied space in target %s = %ld\n", iqn, occsp);

	// open database
	printf("decomm: Opening database %s\n", DBNAME);
	rc = sqlite3_open(DBNAME, &db);
	if(rc) {
		fprintf(stderr, "Can\'t open database %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		printf("FAILED");
		exit(1);
	}

	sprintf(query, "SELECT sum(avspace) FROM Target WHERE iqn != '%s';", iqn);
	rc = sqlite3_exec(db, query, callback, (void *)occsp, &errmsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL Error: %s\n", errmsg);
		sqlite3_free(errmsg);
	}
	// close db
	sqlite3_close(db);

	// put decommissioning code here
	// this includes:
	// removing node in the target db
	// remapping of file parts (cp with changing of db etc.etc.)



	printf("SUCCESS");
	return 0;
}
