#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	char* posi;
	int value;
	 /* TODO: Add more members if necessary */		
} WP;
WP* new_wp();
void free_wp(WP *);
void init_wp_pool();
bool run_wp();
bool point_delete(int);
void point_print();
#endif
