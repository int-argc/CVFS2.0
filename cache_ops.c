/*
    Cache Operations for cache access module
    @author: Jeff
*/

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

#include "global_defs.h"
#include "cmd_exec.h"

#define DBNAME	"cvfs_db"
#define CACHELOC "./srcdir"  // use real cache folder
#define MAX_CACHE_SIZE  3


/*
    increments frequency count of a file, this function assumes that
    the row exists in the database (inserted at initial file write)
*/
void incrementFrequency(string filename) {
    sqlite3 *db;
    char *errmsg = 0;
	int rc;
    string query = "";

    printf("incrementFrequency(): Opening database %s\n", DBNAME);
	rc = sqlite3_open(DBNAME, &db);
	if(rc) {
		fprintf(stderr, "Can\'t open database %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

    sprintf(query, "UPDATE CacheContent SET frequency = frequency + 1 WHERE filename LIKE '%s';", filename);

	rc = sqlite3_exec(db, query, 0, 0, &errmsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL Error: %s\n", errmsg);
		sqlite3_free(errmsg);
	}

	// close db
	sqlite3_close(db);

}

int inCache(string *list, string file) {
    int i;
    for(i = 0; i < MAX_CACHE_SIZE; i++) {
        if (strcmp(list[i], file) == 0) {
            return 1;
        }
    }

    return 0;
}

static int storeCont(void *cont_p, int argc, char **argv, char **colname) {
    int i;
	string comm = "";
    string *contents = cont_p;   // can result in segmentation fault, do not change
    // printf("in store cont\n");

	for (i = 0; i < argc; i++) {
		printf("%s = %s\n", colname[i], argv[i] ? argv[i] : "NULL");
	}

	if(argc != 3) {
		fprintf(stderr, "Error in database table: missing columns\n");
		exit(1);
	} else {
        if(inCache(contents, argv[0])) {
            printf("in cache\n");
        } else {
            printf("not in cache\n");
        }
	}

	printf("\n");
	return 0;
}

void refreshCache() {
    // get files currently in cache
    sqlite3 *db;
    char *errmsg = 0;
	int rc;
    string cachefiles = "", comm = "", comm_out = "", file_list = "", query = "";
    string contents[MAX_CACHE_SIZE];

    sprintf(comm, "ls %s | sed -e ':a;N;$!ba;s/\\n/,/g'", CACHELOC);
    runCommand(comm, comm_out);
    // sprintf(file_list, "\"%s", comm_out);
	// int end = strlen(file_list) - 1;
	// file_list[end] = '\"';
    strcpy(file_list, comm_out);


    // store list of fils in a struct
    char *pch = NULL;
    int i;
    pch = strtok(file_list, ",");       // replace separator if conflict with file name
    printf("printing contents:\n");
    while (pch != NULL) {
        strcpy(contents[i], pch);
        printf("%s\n", contents[i]);
        i++;
        pch = strtok(NULL, ",");
    }

    // open database
	printf("refreshCache(): Opening database %s\n", DBNAME);
	rc = sqlite3_open(DBNAME, &db);
	if(rc) {
		fprintf(stderr, "Can\'t open database %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

    sprintf(query, "SELECT * FROM CacheContent ORDER BY frequency DESC LIMIT %d;", MAX_CACHE_SIZE);

	rc = sqlite3_exec(db, query, storeCont, contents, &errmsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL Error: %s\n", errmsg);
		sqlite3_free(errmsg);
	}
	// close db
	sqlite3_close(db);

}

// temporary driver
int main() {
    // test increment: success!
    refreshCache();

}



/*
cache operation:

this code will be put on read module
so everytime read is recieved, do this func
void incrementFrequency(String filename) {
    update database +1 on frequency of filename
}

on write, (new file -> bahala na yung write mag insert taena)
// note: dapat check size di pde sobrang laki sa cache

// baka dapat naka thread ito or everytime mag increment frequency i call ito
refresh cache() {
    db = select top 10 from cache;
    ls = ls of cache folder
    compare db with ls, if not in ls, copy file to ls
    delete files in ls not in top 10
}




*/
