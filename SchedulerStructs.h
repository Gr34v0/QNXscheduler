/*
 * SchedulerStructs.h
 *
 *  Created on: Oct 7, 2018
 *      Author: Gr34v
 */

#ifndef SCHEDULERSTRUCTS_H_
#define SCHEDULERSTRUCTS_H_

//max 10 children total
#define MAX_CHILDREN 10 //children
#define MAX_NAME_LENGTH 16 //characters
#define SCHEDULER_TICK 1 //second(s)

struct child_params_s{
	char name[MAX_NAME_LENGTH];
	int spin_time;
};


#endif /* SCHEDULERSTRUCTS_H_ */
