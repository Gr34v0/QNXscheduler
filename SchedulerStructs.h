/*
 * SchedulerStructs.h
 *
 *  Created on: Oct 7, 2018
 *      Author: Gr34v
 */

#ifndef SCHEDULERSTRUCTS_H_
#define SCHEDULERSTRUCTS_H_

//max 10 children total
#define MAX_CHILDREN 10
#define MAX_NAME_LENGTH 16

pthread_t children[MAX_CHILDREN];

struct child_params_s{
	char name[MAX_NAME_LENGTH];
	int sleep_time;
	int spin_time;
	int thread_priority;
	pthread_mutex_t *mutex_ptr;
};

struct child_params_s child_buffer[ MAX_CHILDREN ];


#endif /* SCHEDULERSTRUCTS_H_ */
