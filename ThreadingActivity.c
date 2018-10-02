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

//max 10 children total
#define MAX_CHILDREN 10
#define MAX_NAME_LENGTH 16


void* first(void*);
void* second(void*);

pthread_t children[ MAX_CHILDREN ];

struct child_params_s{
	char name[MAX_NAME_LENGTH];
	int sleep_time;
	int spin_time;
	int thread_priority;
	pthread_mutex_t mutex;
};

struct child_params_s child_buffer[ MAX_CHILDREN ];

void* child (void* params) {

	nanospin_calibrate(0);

	struct child_params_s *child_params = params;

	int sleep_time = child_params->sleep_time;
	int spin_time = child_params->spin_time;
	int thread_priority = child_params->thread_priority;
	pthread_mutex_t mutex = child_params->mutex;

	pthread_setname_np(pthread_self(), child_params->name);
	pthread_setschedprio(pthread_self(), thread_priority);

	//sleep(1); //wait for name to be assigned from main thread
	// if this isn't done, the child thread won't always be aware of what it's name is.

	printf("%s is alive\n", child_params->name);

	if(!pthread_mutex_lock(&mutex) != 0){
		printf("pthread_mutex_lock failed for %s\n", child_params->name);
	}

	struct timespec timespecWhen = {0, 25000000}; //25ms

	while(1)
	{
		int i;
		for(i=0; i<((int)spin_time*1000); i+=25)
		{
			int ns = nanospin(&timespecWhen);
			switch(ns){
				case E2BIG :
					printf("Specified delay too large\n");
					break;
				case EINTR :
					printf("A too-high rate of interrupts occurred during the calibration routine\n");
					break;
				case ENOSYS :
					printf("This system's startup-* program didn't initialize the timing information necessary to use nanospin()\n");
					break;
				default :
					break;
			}
		}
		delay(sleep_time*1000);
		printf("%s period complete - %d spin %d sleep\n", child_params->name, child_params->spin_time, child_params->sleep_time);
		fflush(stdout);

	}

	int success = 0;

	pthread_exit(&success);

	return NULL;
}

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

		pthread_mutex_t mutex;
		if(pthread_mutex_init(&mutex, NULL) != 0){
			printf("pthread_mutex_init has failed for child thread %s\n", child_name);
		}

		strcpy(child_buffer[params_count].name, child_name);
		child_buffer[params_count].sleep_time = *sleep;
		child_buffer[params_count].spin_time = *spin;
		child_buffer[params_count].thread_priority = *prio;
		child_buffer[params_count].mutex = mutex;

		pthread_mutex_lock(&mutex);
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


	int keep_alive = 1;
	while(keep_alive){

		char child_to_kill[MAX_NAME_LENGTH];
		memset(child_to_kill, 0x00, MAX_NAME_LENGTH);
		printf("Enter name of child to kill: \n");
		fflush(stdout);
		scanf("%s", child_to_kill);
		printf("Attempting to kill %s\n", child_to_kill);

		int i;
		for( i=0; i < MAX_CHILDREN; i++ )
		{
			char child_name[MAX_NAME_LENGTH];
			pthread_getname_np(children[i], child_name, MAX_NAME_LENGTH);
			if(strcmp(child_name, "main") == 0){
				continue;
			}
			if( strcmp(child_to_kill, child_name) == 0 ){
				if( ThreadDestroy(children[i], 10, NULL) != 0){ //Should use POSIX command?? keep as Microkernel?
					printf("Failed to kill %s\n", child_name);
					fflush(stdout);
				}else{
					alive_children--;
				}
			}
		}

		if(alive_children <= 0){
			keep_alive = 0;
		}
	}

	printf("Children have all been slaughtered.\n");
	fflush(stdout);

	return 0;
}
