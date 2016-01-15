/*
    Symbolic link checking and linking
    @author: Jeff
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "global_defs.h"
#include "cmd_exec.h"

// transfer definitions to global header
#define DBNAME	"cvfs_db"
#define SHARELOC	"./sampleshare"	// use full path please!!!!

// make the soft links here, as of now no threading
static int callback(void *notUsed, int argc, char **argv, char **colname) {
	int i;
	string comm = "";

	for (i = 0; i < argc; i++) {
		printf("%s = %s\n", colname[i], argv[i] ? argv[i] : "NULL");
	}
	// cge bukas mo na gawin jeff
	if(argc != 2) {
		fprintf(stderr, "Error in database table: missing columns\n");
		exit(1);
	} else {
		sprintf(comm, "ln -s \"%s/%s\" \"%s/%s\"", argv[1], argv[0], SHARELOC, argv[0]);
		printf("link with: %s\n", comm);
		// system(comm);
	}

	printf("\n");
	return 0;
}

int main() {
    sqlite3 *db;
	char *errmsg = 0;
	int rc;
	string file_list = "", comm = "", query = "", comm_out = "";

	// get current contents of share folder
	sprintf(comm, "ls %s | sed -e ':a;N;$!ba;s/\\n/\", \"/g'", SHARELOC);
	// printf("comm = %s\n", comm);
	runCommand(comm, comm_out);
	sprintf(file_list, "\"%s", comm_out);
	int end = strlen(file_list) - 1;
	file_list[end] = '\"';

    // open database
	printf("Opening database %s\n", DBNAME);
	rc = sqlite3_open(DBNAME, &db);
	if(rc) {
		fprintf(stderr, "Can\'t open database %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	printf("file_list: \n%s\n", file_list);

	// strcpy(file_list, "\'HUMAART.mp3\'");
    // check volcontent Database
	// option 1 - baka mas mabilis pero kelangan baguhin DB +1 column
    // sprintf(query, "SELECT * FROM VolContent WHERE filename NOT IN (%s);", file_list);
	// option 2 - working as of now
    sprintf(query, "SELECT filename, mountpt FROM VolContent v, Target t WHERE t.tid = v.tid AND filename NOT IN (%s);", file_list);

	rc = sqlite3_exec(db, query, callback, 0, &errmsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL Error: %s\n", errmsg);
		sqlite3_free(errmsg);
	}

	// close db
	sqlite3_close(db);

    return 0;
}
