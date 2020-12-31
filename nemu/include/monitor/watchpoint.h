#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	char str[32];
	int value;
	/* TODO: Add more members if necessary */
} WP;

void init_wp_pool();

WP* new_wp();

void free_wp(WP*);

void delete_wp(int);

void print_wp();

bool check_watchpoints();

#endif
