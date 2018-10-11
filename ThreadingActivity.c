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
 * Spawns two child threads, child 1 sleeps for 5 seconds and child 2 sleeps for 10 seconds.
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
		int* sleep = malloc(sizeof(int));
		int* spin = malloc(sizeof(int));
		int* prio = malloc(sizeof(int));

		printf("Specify name, sleep time, spin time, and thread priority for child %d. Press enter after every entry\n", params_count);

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

		scanf("%d", sleep);
		fflush(stdout);
		printf("Sleep time chosen to be %d\n", *sleep);
		fflush(stdout);

		scanf("%d", spin);
		fflush(stdout);
		printf("Spin time chosen to be %d\n", *spin);
		fflush(stdout);

		scanf("%d", prio);
		fflush(stdout);
		printf("Thread priority chosen to be %d\n", *prio);
		fflush(stdout);


		strcpy(child_buffer[params_count].name, child_name);
		child_buffer[params_count].sleep_time = *sleep;
		child_buffer[params_count].spin_time = *spin;
		child_buffer[params_count].thread_priority = *prio;

		//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

		//child_buffer[params_count].mutex;
		if(pthread_mutex_init(&child_buffer[params_count].mutex, NULL) != EOK){
			printf("pthread_mutex_init has failed for child thread %s\n", child_buffer[params_count].name);
		}else{
			printf("pthread_mutex_init successful for child thread %s\n", child_buffer[params_count].name);
		}

	}

	printf("Starting all children now\n");
	fflush(stdout);

	//create child threads
	int alive_children;

	for(alive_children = 0; alive_children < params_count; alive_children++ ){

		int errcode = pthread_mutex_lock(&child_buffer[alive_children].mutex);
		if( errcode != EOK){
			printf("MAIN: pthread_mutex_lock failed for %s, code %d\n", child_buffer[alive_children].name, errcode);
		}else{
			printf("MAIN: pthread_mutex_lock successful for %s\n", child_buffer[alive_children].name);
		}

		if(pthread_create(&children[alive_children], NULL, child, &child_buffer[alive_children]) !=0){
			printf("pthread_create failed\n");
			exit(2);
		}


	}

	time_t currTime = time(NULL);
	printf("Children created - now waiting - %s\n", ctime(&currTime));


	int keep_alive = 1;
	while(keep_alive){

		char child_to_unlock[MAX_NAME_LENGTH];
		memset(child_to_unlock, 0x00, MAX_NAME_LENGTH);
		printf("Enter name of child to unlock: \n");
		fflush(stdout);
		scanf("%s", child_to_unlock);
		printf("Attempting to unlock %s\n", child_to_unlock);

		int i;
		for( i=0; i < MAX_CHILDREN; i++ )
		{
			char child_name[MAX_NAME_LENGTH];
			pthread_getname_np(children[i], child_name, MAX_NAME_LENGTH);
			if(strcmp(child_name, "main") == 0){
				continue;
			}
			if( strcmp(child_to_unlock, child_name) == 0 ){
				if( pthread_mutex_unlock(&child_buffer[i].mutex) != EOK){
					printf("Failed to unlock %s\n", child_name);
					fflush(stdout);
				}
			}
			sleep(0.5);

			if( pthread_mutex_lock(&child_buffer[i].mutex) != EOK){
				printf("Failed to lock %s\n", child_name);
				fflush(stdout);
			} else{
				printf("Relock successful for %s\n", child_name);
				fflush(stdout);
			}

		}
	}

	printf("Children have all been slaughtered.\n");
	fflush(stdout);

	return 0;
}
