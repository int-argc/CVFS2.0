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
	string iqn = "", query = "", comm = "", comm_out = "", dmount = "", query2 = "";
	strcpy(iqn, argv[1]);
	strcpy(dmount, argv[2]);	// mount point of leaving node
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


	// put decommissioning code here
	// removing node in the target db
	strcpy(query, "");
	sprintf(query, "DELETE FROM Target WHERE iqn = '%s';", iqn);
	rc = sqlite3_exec(db, query, 0, 0, &errmsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL Error: %s\n", errmsg);
		sqlite3_free(errmsg);
		printf("FAILED");
		exit(1);
	}
	// since we cannot use file_map from file_mapping.c, we do another version of it here

	// select all files in dmount
	strcpy(query, "");
	sprintf(query, "SELECT filename FROM VolContent WHERE location = '%s';", dmount);
	sqlite3_stmt *res, *res2;
	const char *tail;
	rc = sqlite3_prepare_v2(db, query, 1000, &res, &tail);
    if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL Error: %s.\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		printf("FAILED");
		exit(1);
    }

	string filename = "", newloc = "";
	string longname = "";	// long filename
	int newtid;		// tid of destinatation target
	double newavspace;	// available space at destination
    while (sqlite3_step(res) == SQLITE_ROW) {
		strcpy(filename, sqlite3_column_text(res, 0));	// file in dmount
		// select target(tid, mountpt) with most available space (same with file_map)
		strcpy(query2, "SELECT avspace, mountpt, tid FROM TARGET WHERE avspace = (SELECT max(avspace) from Target);");
		rc = sqlite3_prepare_v2(db, query2, 1000, &res2, &tail);
	    if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL Error: %s.\n", sqlite3_errmsg(db));
			sqlite3_close(db);
			printf("FAILED");
			exit(1);
	    }
	    if (sqlite3_step(res2) == SQLITE_ROW) {
			newavspace = sqlite3_column_double(res,0);
			strcpy(newloc, sqlite3_column_text(res2, 1));
			newtid = sqlite3_column_int(res2, 2);
			printf("File %s is transfered to %s.\n", filename, newloc);
			strcpy(comm, "");
			sprintf(comm, "mv '%s/%s' '%s/%s'", dmount, filename, newloc, filename);
			system(comm);
		}

	    sqlite3_finalize(res2);

		// update volcontent
		strcpy(query2, "");
		sprintf(query2, "UPDATE VolContent SET location = %s, tid = %d WHERE filename = %s", newloc, newtid, filename);
		rc = sqlite3_exec(db, query2, 0, 0, &errmsg);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL Error on UPDATE VolContent: %s\n", errmsg);
			sqlite3_free(errmsg);
			printf("FAILED");
			exit(1);
		}

		// update cache content
		strcpy(query2, "");
		sprintf(query2, "UPDATE CacheContent SET location = %s WHERE filename = %s", newloc, filename);
		rc = sqlite3_exec(db, query2, 0, 0, &errmsg);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL Error on UPDATE CacheContent: %s\n", errmsg);
			sqlite3_free(errmsg);
			printf("FAILED");
			exit(1);
		}

		// update target to decrease avspace
		strcpy(longname, "");	// di ko alam bakit pero takot ako lagi baka mag segmentation fault so ninunull ko nlng
		sprintf(longname, "%s/%s", newloc, filename);
		FILE *fp = fopen(longname, "rb");
		fseek(fp, 0L, SEEK_END);
		double sz = ftell(fp); //get filesize of file
		newavspace = newavspace - sz;
		sprintf(query2, "UPDATE Target SET avspace = %lf WHERE tid = '%d';", newavspace, newtid);
		rc = sqlite3_exec(db, query2, 0, 0, &errmsg);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL Error on UPDATE Target: %s\n", errmsg);
			sqlite3_free(errmsg);
			printf("FAILED");
			exit(1);
		}
	}



    sqlite3_finalize(res);
	// yehey tapos na?


	// close db
	sqlite3_close(db);
	printf("SUCCESS");
	return 0;
}
