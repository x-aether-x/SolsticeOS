#pragma once
#include <stdint.h>
#define MAX_TASKS 8
struct Task { uint64_t rsp; int state; }; // 0=free 1=ready
extern "C" void switch_task(uint64_t* old_rsp, uint64_t new_rsp);
extern "C" void task_start();
void init_tasking();
int task_create(void (*entry)());
void schedule();