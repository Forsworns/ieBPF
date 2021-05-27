#ifndef INC_TASKS_H_
#define INC_TASKS_H_

k_task_t task_amp; // deal with amp message
k_task_t task_led; // control LED to indicate that program is running
#ifdef BPF_TEST
k_task_t task_bpftest; // bpf bytecode test program
#endif

k_stack_t task_amp_stack[1024];
k_stack_t task_led_stack[1024];
#ifdef BPF_TEST
k_stack_t task_bpftest_stack[1024];
#endif


void task_amp_func();
void task_led_func();
#ifdef BPF_TEST
void task_bpftest_func(void);
#endif


#endif /* INC_TASKS_H_ */
