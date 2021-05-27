#include "main.h"

int main(void)
{
	board_init();
	printf("Board initialization done\r\n");

	k_err_t err;
	tos_knl_init();

	err = tos_task_create(&task_led, "task_led", task_led_func, NULL, 1, task_led_stack, 1024, 20);
	if (err != K_ERR_NONE)
		printf("TencentOS create led task fail! code : %d \r\n", err);
	#ifdef BPF_TEST
	//create bsp test apps
	err = tos_task_create(&task_bpftest, "task_bpftest", task_bpftest_func, NULL, 1, task_bpftest_stack, 1024, 20);
	if (err != K_ERR_NONE)
		printf("TencentOS create bpftest task fail! code : %d \r\n", err);
	#else
	err = tos_task_create(&task_amp, "task_amp", task_amp_func, NULL, 1, task_amp_stack, 1024, 20);
	if (err != K_ERR_NONE)
		printf("TencentOS create amp task fail! code : %d \r\n", err);
	#endif
	

	tos_knl_start();
}
