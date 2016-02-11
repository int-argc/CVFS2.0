/*
    CVFS Inital Configuration
    @author: Aidz <3 Jeff           :">
    requires:
        open-iscsi
        nmap
        sqlite3
        libsqlite3-dev
    uses:
        cmd_exec.c
        volman.c
*/

#include <stdio.h>
#include <string.h>
#include "global_defs.h"
#include "cmd_exec.h"
#include "volman.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <sqlite3.h>

#define TARGET_FILE "target_list.txt"
#define GBSTR  "GiB"
#define MBSTR  "MB"    // assumption
#define TBSTR  "TB"

double toBytes(string sUnit) {
    char *ptr;

    ptr = strtok(sUnit, " ");
    double val = atof(ptr);
    ptr = strtok(NULL, " ");
    if (strcmp(ptr, GBSTR) == 0) {
        val *= 1000000000;
    } else if (strcmp(ptr, MBSTR) == 0) {
        val *= 1000000;
    } else if (strcmp(ptr, TBSTR) == 0) {
        val *= 1000000000000;
    }
    // printf("val = %f\n", val);
    return val;
}

int main() {

    // get network information of initiator
    string ip, netmask, command, alltargetsStr, currtarget, iqn, sendtargets,sql;
    string disklist, assocvol, mountpt, avspace, command1, sql1;

    //sqlite3 information
    sqlite3 *db;
    int rc = 0;

    //socket information
    int portno = 3260;
    int sockfd;
    struct sockaddr_in serv_addr;

    //open sqlite3 db
    rc = sqlite3_open("cvfs_db",&db);
    if (rc){
       printf("Cannot open database.");
       exit(0);
    }

    // what if interface is not eth0? what if eth1...?
    runCommand("ip addr show eth0 | grep \'inet \' | awk \'{print $2}\' | cut -f1 -d\'/\'", ip);
    runCommand("ip addr show eth0 | grep \'inet \' | awk \'{print $2}\' | cut -f2 -d\'/\'", netmask);
    printf("*****  Network information  *****\n");
    ip[strlen(ip)] = '\0';
    netmask[strlen(netmask)] = '\0';
    printf("IP address: %s/%s\n\n", ip, netmask);


    // do nmap for active hosts with port 3260 open
    printf("Scanning network...\n");
    sprintf(command, "nmap %s%s%s -n -sP | grep \"report\" | awk '{print $5}'", ip, "/", netmask);
    runCommand(command, alltargetsStr);

  // discover iscsi targets on hosts scanned successfully
    char *ptr;
    ptr = strtok(alltargetsStr, "\n");

    while(ptr != NULL) {

	memset(&serv_addr,0,sizeof(serv_addr));
        sockfd = socket(AF_INET,SOCK_STREAM,0);

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(ptr);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
	    printf("%s is not a target.\n", ptr);
	} else {
	    printf("%s is a target.\n", ptr);
	    sprintf(sendtargets,"iscsiadm -m discovery -t sendtargets -p %s | awk '{print $2}'",ptr);
	    runCommand(sendtargets,iqn);
            printf("%s\n", iqn);

            sprintf(sql,"insert into Target(ipadd,iqn) values ('%s','%s');",ptr,iqn);

	    rc = sqlite3_exec(db,sql,0,0,0);
	    if (rc != SQLITE_OK){
	        printf("Did not insert successfully!");
                exit(0);
            }
	    strcpy(sendtargets,"");
            strcpy(sql,"");
        }


        ptr = strtok(NULL, "\n");
	close(sockfd);
    }

    // dapat ata meron tayo -m node -o delete para di matagal login (since na reretain dating mga target)
    // -m node --login
    printf("\n\nLogging in to targets...");
    system("iscsiadm -m node --login");
    sleep(10);
    printf("\n\nAvailable partitions written to file \"%s\"\n", AV_DISKS);
    sprintf(command, "cat /proc/partitions > %s", AV_DISKS);
    system(command);
    system("cat /proc/partitions");

    //system("cat AvailableDisks.txt | grep sd[b-z] | awk '{print $4}'");

    // volume manager: initialize and create volumes
    makeVolume(0);

    runCommand(disklist,"cat AvailableDisks.txt | grep sd[b-z] | awk '{print $4}'");

    char *ptr1;
    int counter = 1;    // to aidz: di ako sure dito ah.. pano kung nag delete then init_conf sure ba na 1 lagi tapos sunod sunod?
    ptr1 = strtok(disklist,"\n");

    while(ptr1 != NULL){
       strcat(assocvol,"/dev/vg");
       strcat(assocvol,disklist);
       strcat(assocvol,"/lv");
       strcat(assocvol,disklist);

       strcat(mountpt,"/mnt/lv");
       strcat(mountpt,disklist);

       sprintf(command1,"lvdisplay %s | grep 'LV Size' | awk '{print $3,$4}'",assocvol);
       runCommand(avspace,command1);

       // edit here not sure if working (assume: avspace = "12.3 GiB")
       double space_bytes = toBytes(avspace);
       sprintf(sql1,"insert into Target(assocvol,mountpt,avspace) values ('%s','%s',%f) where tid = %d",assocvol,mountpt,space_bytes,counter);

       rc = sqlite3_exec(db,sql1,0,0,0);

       if (rc != SQLITE_OK){
           printf("Did not insert successfully!");
           exit(0);
       }

       strcpy(assocvol,"");
       strcpy(mountpt,"");
       strcpy(avspace,"");
       counter++;
       ptr1 = strtok(NULL,"\n");
    }

    printf("\n\nInitialization finished");

    return 0;
}
