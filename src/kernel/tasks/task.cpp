#include "task.h"
#include "memory.h"
Task tasks[MAX_TASKS];
static int current = 0;
static bool tasking_on = false;
void init_tasking() { tasks[0].state = 1; tasking_on = true; } // task 0 = boot flow
static void task_exit() { tasks[current].state = 0; for (;;) { asm volatile("sti; hlt"); } }
int task_create(void (*entry)()) {
    for (int i = 1; i < MAX_TASKS; i++) if (tasks[i].state == 0) {
        uint64_t* stack = (uint64_t*)((uint8_t*)pmm_alloc() + 4096);
        *--stack = (uint64_t)task_exit;   // entry's return address
        *--stack = (uint64_t)entry;       // popped by task_start's ret
        *--stack = (uint64_t)task_start;  // first ret: sti, then jump entry
        for (int j = 0; j < 6; j++) *--stack = 0; // rbx rbp r12-r15
        tasks[i].rsp = (uint64_t)stack; tasks[i].state = 1;
        return i;
    }
    return -1;
}
void schedule() {
    if (!tasking_on) return;
    int next = current;
    do { next = (next + 1) % MAX_TASKS; } while (tasks[next].state != 1 && next != current);
    if (next == current) return;
    int old = current; current = next;
    switch_task(&tasks[old].rsp, tasks[next].rsp);
}