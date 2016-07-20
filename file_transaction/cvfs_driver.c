
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>

#include "../Global/global_definitions.h"
#include "../disk_pooling/file_presentation.h"
#include "../disk_pooling/initial_configurations.h"
#include "watch_share.h"
#include "watch_dir.h"

#define THREADCNT 2

// shows help message and exit
void show_help() {
    printf("Usage:\n");
    printf("\tcvfs_driver [init]\n");
    printf("use init to initialize system");
    exit(1);
}

int main(int argc, char *argv[]) {
    system("clear");
    // open logging
    openlog("cvfs2", LOG_PID|LOG_CONS, LOG_USER);

    if(argc > 2){
		printf("FATAL: Too much arguments.\n");
        show_help();
	} else if(argc == 2) {
        if(strcmp(argv[1], "init") == 0) {      // initialize and exit
            initialize();
            exit(0);
        } else {
            printf("FATAL: Wrong argument.\n");
    		show_help();
        }
    }

    pthread_t t[THREADCNT];
    int i;

    int flag = 0; //initial value of flag 
    FILE *fp = fopen("random.txt", "w");
    fprintf(fp, "%d", flag);
    fclose(fp);

    while(1) {
        //pthread_create(&t[0], NULL, create_link, NULL);
        pthread_create(&t[0], NULL, watch_temp, NULL);
        pthread_create(&t[1], NULL, watch_share, NULL);
        // pthread_create(&t[2], NULL, create_link, NULL);

        for (i = 0; i < THREADCNT; i++) {
            pthread_join(t[i], NULL);
        }
    }

}
