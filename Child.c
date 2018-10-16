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
	int spin_time = child_params->spin_time;

	pthread_setname_np(pthread_self(), child_params->name);

	sleep(1);

	printf("%s is alive\n", child_params->name);

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
		printf("%s period complete - %d spin\n", child_params->name, child_params->spin_time);
		fflush(stdout);

	}

	int success = 0;

	pthread_exit(&success);

	return NULL;
}
