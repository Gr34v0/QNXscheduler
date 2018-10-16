#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/trace.h>
#include <time.h>
#include <sys/syspage.h>
#include <sys/neutrino.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include <SchedulerStructs.h>
#include <child.h>


struct child_params_s child_buffer[ MAX_CHILDREN ];
pthread_t children[ MAX_CHILDREN ];


int inputNumberOfChildren(){
	int number_of_children;
	printf("How many children would you like to spawn? Max %d: ", MAX_CHILDREN);
	scanf("%d", &number_of_children);
	fflush(stdout);

	if(number_of_children > MAX_CHILDREN){
		printf("Can not have more than %d children. Try again.\n", MAX_CHILDREN);
		fflush(stdout);
		number_of_children = inputNumberOfChildren();
	}

	return number_of_children;
}


/*
 * Spawns child threads, sleeps for a "tick" period of time, swaps
 * Main thread prints time stamps when children awaken.
 */
int main(int argc, char *argv[]) {

	nanospin_calibrate(0);
	memset(children, 0x00, sizeof(children));

	pthread_setname_np(pthread_self() ,"main");
	pthread_setschedprio(pthread_self(), 10);

	int children_count = inputNumberOfChildren();

	int params_count;

	for( params_count = 0; params_count<children_count; params_count++)
	{
		char child_name[MAX_NAME_LENGTH];
		memset(child_name, 0x00, MAX_NAME_LENGTH);
		int* spin = malloc(sizeof(int));

		printf("Specify name and spin time for child %d. Press enter after every entry\n", params_count);

		scanf("%s", child_name);
		fflush(stdout);
		child_name[MAX_NAME_LENGTH - 1] = '\0';
		printf("Name chosen to be %s\n", child_name);
		fflush(stdout);

		int tmpcounter;
		for(tmpcounter=0; tmpcounter < MAX_CHILDREN; tmpcounter++ )
		{
			char existing_child_name[MAX_NAME_LENGTH];
			pthread_getname_np(children[tmpcounter], existing_child_name, MAX_NAME_LENGTH);

			int cmpresult = strcmp(existing_child_name, child_name);
			if( !cmpresult ){
				printf("Thread name should be unique: %d - %s - %s\n", cmpresult, existing_child_name, child_name);
				fflush(stdout);
				params_count--;
			}
		}

		scanf("%d", spin);
		fflush(stdout);
		printf("Spin time chosen to be %d\n", *spin);
		fflush(stdout);

		strcpy(child_buffer[params_count].name, child_name);
		child_buffer[params_count].spin_time = *spin;

	}

	printf("Starting all children now\n");
	fflush(stdout);

	//create child threads
	int alive_children;

	for(alive_children = 0; alive_children < params_count; alive_children++ ){

		if(pthread_create(&children[alive_children], NULL, child, &child_buffer[alive_children]) !=0){
			printf("pthread_create failed\n");
			exit(2);
		}

	}

	time_t currTime = time(NULL);
	printf("Children created - now waiting - %s\n", ctime(&currTime));

	sleep(5);

	int keep_alive = 1;
	int thread_indexer = 0;

	while(keep_alive){

		sleep(SCHEDULER_TICK);

		pthread_setschedprio(children[thread_indexer%children_count], 8);
		thread_indexer++;
		pthread_setschedprio(children[thread_indexer%children_count], 9);

	}

	printf("Children have all been slaughtered.\n");
	fflush(stdout);

	return 0;
}
