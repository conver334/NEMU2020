#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

WP* new_wp() {
	if(free_ == NULL) {
		printf("no enough space for new watchpoint\n");
		assert(0);
	}
	WP* cur = free_;
	free_ = free_ -> next;
	cur -> next = head;
	head = cur;
	printf("wp_pool id:%d is available\n", cur -> NO);
	return cur;
}

void free_wp(WP* wp) {
	WP* cur = head;
	while(cur != NULL) {
		if(cur -> next == wp) break;
		else cur = cur -> next;
	}
	if(cur == NULL && head != wp) {
		printf("this watchpoint is not used.\n");
		return ;
	}
	else if(head == wp) {
		head = head -> next;
	}
	else {
		cur -> next = wp -> next;
	}
	wp -> next = free_;
	free_= wp;
	return ;
}

void delete_wp(int number) {
	WP* wp = head;
	while(wp != NULL){
		printf("fuck: %d\n", wp -> NO);
		if(wp -> NO == number) break;
		else wp = wp -> next;
	}
	if(wp == NULL){
		printf("the watchpoint not in used\n"); return ;
	}
	WP* cur = head;
	while(cur != NULL) {
		if(cur -> next == wp) break;
		else cur = cur -> next;
	}
	if(cur == NULL && head != wp) {
		printf("this watchpoint is not used.\n");
		return ;
	}
	else if(head == wp) {
		head = head -> next;
	}
	else {
		cur -> next = wp -> next;
	}
	wp -> next = free_;
	free_= wp;
	return ;
}

void print_wp(){
	WP* cur = head;
	while(cur != NULL){
		int val; bool ok = true;
		val = expr(cur -> str, &ok);
		printf("wp id: %d, expr: %s, value: %d\n", cur -> NO, cur -> str, val);
		cur = cur -> next;
	}
	return;
}

bool check_watchpoints() {
	WP* cur = head;
	bool changed = false;
	while(cur != NULL){
		bool ok = true;
		int cur_val = expr(cur -> str, &ok);
		if(cur_val != cur -> value){
			printf("watchpoints id: %d has changed its value, from %d to %d.\n", cur -> NO, cur ->value, cur_val);
			cur -> value = cur_val;
			changed = true;
		}
		cur = cur -> next;
	}
	return changed;
}

/* TODO: Implement the functionality of watchpoint */


