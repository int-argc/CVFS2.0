# makefile for CVFS2.0
# compile system with 'make'

CC = gcc
LIBS = -lpthread -lsqlite3
MAIN = cvfs_driver

all: $(MAIN) add_target decomm

$(MAIN): file_transaction/cvfs_driver.c cache_operation.o file_presentation.o initial_configurations.o file_striping.o file_mapping.o file_assembly.o make_volumes.o cmd_exec.o watch_dir.o watch_share.o #target_stat.o
	$(CC) file_transaction/cvfs_driver.c cache_operation.o file_presentation.o initial_configurations.o file_striping.o file_mapping.o file_assembly.o make_volumes.o cmd_exec.o watch_dir.o watch_share.o $(LIBS) -DTHREADSAFE=1 -o file_transaction/$(MAIN)

cache_operation.o: cache_access/cache_operation.c
	$(CC) -c cache_access/cache_operation.c

file_presentation.o: disk_pooling/file_presentation.c
	$(CC) -c disk_pooling/file_presentation.c

initial_configurations.o: disk_pooling/initial_configurations.c
	$(CC) -c disk_pooling/initial_configurations.c

file_striping.o: file_striping/file_striping.c
	$(CC) -c file_striping/file_striping.c

file_mapping.o: volume_management/file_mapping.c
	$(CC) -c volume_management/file_mapping.c

file_assembly.o: volume_management/file_assembly.c
	$(CC) -c volume_management/file_assembly.c

make_volumes.o: volume_management/make_volumes.c
	$(CC) -c volume_management/make_volumes.c

cmd_exec.o: Utilities/cmd_exec.c
	$(CC) -c Utilities/cmd_exec.c

watch_dir.o: file_transaction/watch_dir.c
	$(CC) -c file_transaction/watch_dir.c

watch_share.o: file_transaction/watch_share.c
	$(CC) -c file_transaction/watch_share.c

target_stat.o: file_recovery/target_stat.c
	$(CC) -c file_recovery/target_stat.c

add_target: WebSystem/add_target.c make_volumes.o cmd_exec.o initial_configurations.o
	$(CC) WebSystem/add_target.c make_volumes.o cmd_exec.o  initial_configurations.o $(LIBS) -o file_transaction/add_target

decomm: WebSystem/decomm.c make_volumes.o cmd_exec.o
	$(CC) WebSystem/decomm.c cmd_exec.o -lsqlite3 -o file_transaction/decomm

# this line just for solo recov
target_stat: file_recovery/target_stat.c
	$(CC) file_recovery/target_stat.c $(LIBS) -O file_recovery/target_stat
