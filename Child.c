/*
 * child.c
 *
 *  Created on: Oct 7, 2018
 *      Author: Gr34v
 */

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

void* child (void* params) {

	nanospin_calibrate(0);

	struct child_params_s *child_params = params;

	int sleep_time = child_params->sleep_time;
	int spin_time = child_params->spin_time;
	int thread_priority = child_params->thread_priority;
	//child_params->mutex;

	pthread_setname_np(pthread_self(), child_params->name);
	pthread_setschedprio(pthread_self(), thread_priority);



	sleep(1); //wait for name to be assigned from main thread
	// if this isn't done, the child thread won't always be aware of what it's name is.

	printf("%s is alive\n", child_params->name);

	struct timespec timespecWhen = {0, 25000000}; //25ms

	while(1)
	{
		int errcode = pthread_mutex_lock(&child_params->mutex);
		if( errcode != EOK){
			printf("CHILD %s: pthread_mutex_lock failed, code %d\n", child_params->name, errcode);
		}else{
			printf("CHILD %s: pthread_mutex_lock successful\n", child_params->name);
		}

		int errcode2 = pthread_mutex_unlock(&child_params->mutex);
		if( errcode2 != EOK){
			printf("CHILD %s: pthread_mutex_unlock failed, code %d\n", child_params->name, errcode2);
		}else{
			printf("CHILD %s: pthread_mutex_unlock successful\n", child_params->name);
		}


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
