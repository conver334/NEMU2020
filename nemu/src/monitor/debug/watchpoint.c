#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;
WP* new_wp(){
	if(free_ == NULL){
		printf("no enough space for new watchpoint\n");
		assert(0);
	}
	printf("hahha");
	Log("hahah\n");
	WP* fir_free = free_;
	free_ = free_->next;
	fir_free->next = head;
	head = fir_free;
	return fir_free;
}
void free_wp(WP *wp){
	bool flag=false;
	if(head==wp){
		head=head->next;
		flag=true;
	}
	else{
		WP* i=head;
		while(i->next != wp){
			i = i->next;
		}
		i->next = wp->next;
		flag=true;
	}
	if(flag){
		wp->next = free_;
		free_ = wp;
	}
	else{
		printf("this watchpoint doesn't exit!\n");
	}
}
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
bool run_wp(){
	WP* i = head;
	bool exp_success=true, change=false;
	int value_now;
	for(; i != NULL; i = i->next){
		value_now = expr(i->posi,&exp_success) ;
		if(value_now != i->value){
			printf("Hardware watchpoint %d: %s\n",i->NO,i->posi);
			printf("Old value = %x\nNew value = %x\n",i->value,value_now);
			i->value = value_now;
			change = true;
		}
	}
	return change;
}
bool point_delete(int rank){
	if(head == NULL)return 0;
	if(head -> NO == rank){
		head=head->next;return 1;
	}
	WP* fr= head; 
	WP* i = head->next;
	for(; i != NULL;fr = i, i = i->next){
		if( i->NO == rank){
			fr->next = i->next;
			return 1;
		}
	}
	return 0;
}
void point_print(){
	WP* i = head;
	for(; i != NULL; i = i->next){
		printf("Hardware watchpoint %d: %s = %x\n",i->NO,i->posi,i->value);
	}
}
/* TODO: Implement the functionality of watchpoint */


