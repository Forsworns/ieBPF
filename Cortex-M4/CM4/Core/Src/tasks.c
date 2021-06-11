/*
 * Copyright 2015 Big Switch Networks, Inc
 * Copyright 2017 Google Inc.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "main.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include "elf_loader.h" // included in the OpenAMP

#ifdef BPF_TEST
#include <unistd.h>
#include "testprog.h" // adding bpf test apps
#endif

void task_led_func()
{
	while (1)
	{
		HAL_GPIO_TogglePin(GPIOZ, GPIO_PIN_5);
		HAL_GPIO_TogglePin(GPIOZ, GPIO_PIN_6);
		HAL_GPIO_TogglePin(GPIOZ, GPIO_PIN_7);
		tos_task_delay(1000);
	}
}

#define MAX_BUFFER_SIZE RPMSG_BUFFER_SIZE
#define MAX_BPF_LENGTH

IPCC_HandleTypeDef hipcc;

VIRT_UART_HandleTypeDef huart0;
VIRT_UART_HandleTypeDef huart1;

__IO FlagStatus channel0_rx_flag = RESET;
uint8_t channel0_rx[MAX_BUFFER_SIZE];
uint16_t channel0_rx_len = 0;

__IO FlagStatus channel1_rx_flag = RESET;
uint8_t channel1_rx[MAX_BUFFER_SIZE];
uint16_t channel1_rx_len = 0;

char CFG_JIT[] = "JIT";
char CFG_INTERPRET[] = "INTERPRET";
uint8_t bpf_code[MAX_BUFFER_SIZE] = {0};
uint16_t bpf_code_len = 0;
bool jit = false;

void uart0_rx_callback(VIRT_UART_HandleTypeDef *huart);
void uart1_rx_callback(VIRT_UART_HandleTypeDef *huart);
void amp_init();
uint64_t bpf_vm();
static void register_functions(struct ubpf_vm *vm);

void task_amp_func()
{
	amp_init();
	while (1)
	{
		// maybe put these codes in the callbacks above?
		OPENAMP_check_for_message();
		if (channel0_rx_flag)
		{
			channel0_rx_flag = RESET;
			if (strncmp((char *)channel0_rx, CFG_JIT, strlen(CFG_JIT)) == 0)
			{
				jit = true;
				// simply send the message back
				VIRT_UART_Transmit(&huart0, channel0_rx, channel0_rx_len);
			}
			else if (strncmp((char *)channel0_rx, CFG_INTERPRET, strlen(CFG_INTERPRET)) == 0)
			{
				jit = false;
				VIRT_UART_Transmit(&huart0, channel0_rx, channel0_rx_len);
			}
			else
			{
				// by default, it should be BPF codes now, either binary elf or binary byte code from ubpf python scripts
				// now it only supports program length up to MAX_BUFFER_SIZE
				// consider relax this constraint, or seperately send different segments?
				memcpy(bpf_code, channel0_rx, channel0_rx_len);
				bpf_code_len = channel0_rx_len;
				uint64_t result = bpf_vm();
				sprintf((char *)channel0_rx, "Result is %lu\n", result); // seems not support %llu
				VIRT_UART_Transmit(&huart0, channel0_rx, strlen(channel0_rx));
			}
		}
		// currently, ttyRPMSG0 and ttyRPMSG1 send the same data,
		// maybe used for control and data seperately like what `FTP` does
		else if (channel1_rx_flag)
		{
			channel1_rx_flag = RESET;
			if (strcmp((char *)channel1_rx, CFG_JIT) == 0)
			{
				jit = true;
				VIRT_UART_Transmit(&huart1, channel1_rx, channel1_rx_len);
			}
			else if (strcmp((char *)channel1_rx, CFG_INTERPRET) == 0)
			{
				jit = false;
				VIRT_UART_Transmit(&huart1, channel1_rx, channel1_rx_len);
			}
			else
			{
				memcpy(bpf_code, channel1_rx, channel1_rx_len);
				bpf_code_len = channel1_rx_len;
				uint64_t result = bpf_vm();
				sprintf((char *)channel1_rx, "Result is %lu\n", result);
				VIRT_UART_Transmit(&huart1, channel1_rx, strlen(channel1_rx));
			}
		}else{
			// tos_task_delay(100); // only for validation of multi-task RTOS. without delay, LED task above will not be carried
		}
	}
}

void uart0_rx_callback(VIRT_UART_HandleTypeDef *huart)
{
	channel0_rx_len = huart->RxXferSize < MAX_BUFFER_SIZE ? huart->RxXferSize : MAX_BUFFER_SIZE - 1;
	memcpy(channel0_rx, huart->pRxBuffPtr, channel0_rx_len);
	channel0_rx_flag = SET;
}

void uart1_rx_callback(VIRT_UART_HandleTypeDef *huart)
{

	channel1_rx_len = huart->RxXferSize < MAX_BUFFER_SIZE ? huart->RxXferSize : MAX_BUFFER_SIZE - 1;
	memcpy(channel1_rx, huart->pRxBuffPtr, channel1_rx_len);
	channel1_rx_flag = SET;
}

void amp_init()
{
	if (VIRT_UART_Init(&huart0) != VIRT_UART_OK)
	{
		Error_Handler();
	}

	if (VIRT_UART_Init(&huart1) != VIRT_UART_OK)
	{
		Error_Handler();
	}

	if (VIRT_UART_RegisterCallback(&huart0, VIRT_UART_RXCPLT_CB_ID, uart0_rx_callback) != VIRT_UART_OK)
	{
		Error_Handler();
	}
	if (VIRT_UART_RegisterCallback(&huart1, VIRT_UART_RXCPLT_CB_ID, uart1_rx_callback) != VIRT_UART_OK)
	{
		Error_Handler();
	}
}

uint64_t bpf_vm()
{
	uint64_t result = 0;
	size_t mem_len = 0; // currently don't consider the mem part
	void *mem = NULL;
	/* 	
	tos supports elf, but it seems only support loading from file system,
	have included the related elf and fs components,
	OpenAMP also includes `elf_loader.h`, so the original codes may work
	but currently, we only focus on the ascii ones
	*/
	struct ubpf_vm *vm = ubpf_create();
	if (!vm)
	{
		fprintf(stderr, "Failed to create VM\n");
		return 1;
	}

	register_functions(vm);

	/* 
	* Check the first four chars to detect whether it is an elf file
	* The ELF magic corresponds to an RSH instruction with an offset,
	* which is invalid.
	*/
	bool elf = bpf_code_len >= SELFMAG && !memcmp(bpf_code, ELFMAG, SELFMAG);
	char *errmsg;
	int rv;
	if (elf)
	{
		rv = ubpf_load_elf(vm, bpf_code, bpf_code_len, &errmsg);
	}
	else
	{
		rv = ubpf_load(vm, bpf_code, bpf_code_len, &errmsg);
	}

	if (rv < 0)
	{
		printf("Failed to load code: %s\n", errmsg);
		ubpf_destroy(vm);
		return 0;
	}

	if (jit)
	{
		ubpf_jit_fn fn = ubpf_compile(vm, &errmsg);
		if (fn == NULL)
		{
			printf("Failed to compile: %s\n", errmsg);
			return 0;
		}
		printf("Run the JIT compiled code...\n");
		result = fn(mem, mem_len);
	}
	else
	{
		printf("Interpret the code...\n");
		result = ubpf_exec(vm, mem, mem_len);
	}

	ubpf_destroy(vm);

	return result;
}

static uint64_t
gather_bytes(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e)
{
	return ((uint64_t)a << 32) |
		   ((uint32_t)b << 24) |
		   ((uint32_t)c << 16) |
		   ((uint16_t)d << 8) |
		   e;
}

static void *
memfrob(void *s, size_t n)
{
	char *p = (char *)s;

	while (n-- > 0)
		*p++ ^= 42;

	return s;
}

static void
trash_registers(void)
{
	/* Overwrite all caller-save registers */
	asm(
		"mov r0, #0xf0;"
		"mov r1, #0xf1;"
		"mov r2, #0xf2;"
		"mov r3, #0xf3;"
		"mov r12, #0xf4;"
		"mov r14, #0xf6;");
}

static uint32_t
sqrti(uint32_t x)
{
	return sqrt(x);
}

static void
register_functions(struct ubpf_vm *vm)
{
	ubpf_register(vm, 0, "gather_bytes", gather_bytes);
	ubpf_register(vm, 1, "memfrob", memfrob);
	ubpf_register(vm, 2, "trash_registers", trash_registers);
	ubpf_register(vm, 3, "sqrti", sqrti);
	ubpf_register(vm, 4, "strcmp_ext", strcmp);
}

#ifdef BPF_TEST
static void * readtestprog(size_t * codelen)
{
	void *data = calloc(testprog_len, 1);
	memcpy(data,testprog,testprog_len);
	*codelen = testprog_len;
	return data;

}

static void *readfile(const char *path, size_t maxlen, size_t *len)
{
    FILE *file;
    if (!strcmp(path, "-")) {
        file = fdopen(STDIN_FILENO, "r");
    } else {
        file = fopen(path, "r");
    }

    if (file == NULL) {
        fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
        return NULL;
    }

    void *data = calloc(maxlen, 1);
    size_t offset = 0;
    size_t rv;
    while ((rv = fread(data+offset, 1, maxlen-offset, file)) > 0) {
        offset += rv;
    }

    if (ferror(file)) {
        fprintf(stderr, "Failed to read %s: %s\n", path, strerror(errno));
        fclose(file);
        free(data);
        return NULL;
    }

    if (!feof(file)) {
        fprintf(stderr, "Failed to read %s because it is too large (max %u bytes)\n",
                path, (unsigned)maxlen);
        fclose(file);
        free(data);
        return NULL;
    }

    fclose(file);
    if (len) {
        *len = offset;
    }
    return data;
}

int bpf_test_apps(void)
{
	    bool jit = false;
	    size_t code_len;	
		const char *mem_filename = NULL;	    
	    void *code = readtestprog(&code_len);
	    if (code == NULL) {
	        return 1;
	    }

		size_t mem_len = 0;
	    void *mem = NULL;
	    if (mem_filename != NULL) {
	        mem = readfile(mem_filename, 1024*1024, &mem_len);
	        if (mem == NULL) {
	            return 1;
	        }
	    }
		
	    struct ubpf_vm *vm = ubpf_create();
	    if (!vm) {
	        fprintf(stderr, "Failed to create VM\n");
	        return 1;
	    }

	    register_functions(vm);

	    /*
	     * The ELF magic corresponds to an RSH instruction with an offset,
	     * which is invalid.
	     */
	    bool elf = code_len >= SELFMAG && !memcmp(code, ELFMAG, SELFMAG);

	    char *errmsg;
	    int rv;
	    if (elf) {
		rv = ubpf_load_elf(vm, code, code_len, &errmsg);
	    } else {
		rv = ubpf_load(vm, code, code_len, &errmsg);
	    }


	    free(code);

	    if (rv < 0) {
	        fprintf(stderr, "Failed to load code: %s\n", errmsg);
	        free(errmsg);
	        ubpf_destroy(vm);
	        return 1;
	    }

	    uint64_t ret;

	    if (jit) {
	        ubpf_jit_fn fn = ubpf_compile(vm, &errmsg);
	        if (fn == NULL) {
	            fprintf(stderr, "Failed to compile: %s\n", errmsg);
	            free(errmsg);
	            return 1;
	        }
	        ret = fn(mem, mem_len);
	    } else {
	        ret = ubpf_exec(vm, mem, mem_len);
	    }

	    printf("0x%"PRIx64"\n", ret);

	    ubpf_destroy(vm);

	    return 0;
	}


void task_bpftest_func(void)
{
	bpf_test_apps();	
}
#endif

