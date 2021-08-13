#ifndef INC_TASKS_H_
#define INC_TASKS_H_

k_task_t task_amp; // deal with amp message
k_task_t task_led; // control LED to indicate that program is running

k_stack_t task_amp_stack[1024];
k_stack_t task_led_stack[1024];

void task_amp_func();
void task_led_func();


#endif /* INC_TASKS_H_ */
