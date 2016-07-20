/*
    Manual ISCSI configuration
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <sqlite3.h>
#include "../Global/global_definitions.h"
#include "../Utilities/cmd_exec.h"
#include "../volume_management/make_volumes.h"

int main(int argc, char *argv[]) {
	int rc = 0;
	sqlite3 *db;

	rc = sqlite3_open(DBNAME, &db);


    if(argc != 2){
		printf("FATAL: Program takes exactly 1 argument.\n");
		printf("Usage:\n");
		printf("\tadd_target <IP address of target>\n");
		printf("FAILED");
		exit(1);
	}
    openlog("cvfs2", LOG_PID|LOG_CONS, LOG_USER);
    String ipadd = "";
    strcpy(ipadd, argv[1]);
    syslog(LOG_INFO, "add_target: Target with IP address %s will be added.\n", ipadd);
    printf("Target IP Address: %s\n", ipadd);

    String comm, out;
    sprintf(comm, "nmap %s -p3260 --open | grep open", ipadd);
    runCommand(comm, out);
    if (strcmp(out, "") == 0) {
        printf("Error: Invalid Target (%s)\n", ipadd);
        printf("FAILED");
        exit(1);
    }

    String iqn = "", login = "", sql = "";
    // target is valid, proceed to discovery
    sprintf(comm, "iscsiadm -m discovery -t sendtargets -p %s | awk '{print $2}'", ipadd);
    //system(comm);
	printf("comm = %s\n", comm);
    runCommand(comm, iqn);
   printf("IQN := %s\n", iqn);
    sprintf(login, "iscsiadm -m node -l -T %s -p %s:3260", iqn, ipadd);
    printf("LOGIN := %s\n", login);
    system(login);
        sleep(3);
        sprintf(sql,"insert into Target(ipadd,iqn) values ('%s','%s');",ipadd,iqn);

        printf("** initconf, sql = %s\n", sql);

        rc = sqlite3_exec(db,sql,0,0,0);
        if (rc != SQLITE_OK){
            printf("\nDid not insert successfully!\n");
            exit(0);
        }
        strcpy(sql,"");
        strcpy(iqn, "");

    //printf("\n\nLogging in to targets...");
    //system("iscsiadm -m node --login");
    // sleep(5);

    // volume manager: initialize and create volumes
    makeVolumeAdd(ipadd);
    printf("\n\nAvailable partitions written to file \"%s\"", AV_DISKS);
    /* overwrites previous, para pag next update, i exempt na yung updated disk */
    sprintf(comm, "cat /proc/partitions | awk '{print $4}' | grep 'sd[b-z]' > %s", AV_DISKS);
    system(comm);

    printf("\n\nAdding target finished :)\n");
    syslog(LOG_INFO, "add_target: Adding target finished.\n");
sqlite3_close(db);
    printf("SUCCESS");
    return 0;
}


// check if valid target (discovery, check for 3260)
// initial config will check for available targets in the network
// automatic configuration of volumes
// update will be manual, admin will input IP / target IQN
