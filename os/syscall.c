#include "syscall.h"
#include "defs.h"
#include "loader.h"
#include "syscall_ids.h"
#include "timer.h"
#include "trap.h"
#include "proc.h"

uint64 sys_write(int fd, uint64 va, uint len)
{
	debugf("sys_write fd = %d va = %x, len = %d", fd, va, len);
	if (fd != STDOUT)
		return -1;
	struct proc *p = curr_proc();
	char str[MAX_STR_LEN];
	int size = copyinstr(p->pagetable, str, va, MIN(len, MAX_STR_LEN));
	debugf("size = %d", size);
	for (int i = 0; i < size; ++i) {
		console_putchar(str[i]);
	}
	return size;
}

__attribute__((noreturn)) void sys_exit(int code)
{
	exit(code);
	__builtin_unreachable();
}

uint64 sys_sched_yield()
{
	yield();
	return 0;
}

uint64 sys_gettimeofday(TimeVal *val, int _tz) // TODO: implement sys_gettimeofday in pagetable. (VA to PA)
{
	// YOUR CODE
	struct proc *p = curr_proc();
	char src[sizeof(TimeVal)];

	TimeVal* time = (TimeVal *)src;
	uint64 cycle = get_cycle();
	time->sec = cycle / CPU_FREQ;
	time->usec = (cycle % CPU_FREQ) * 1000000 / CPU_FREQ;

	if (copyout(p->pagetable, (uint64)val, src, sizeof(TimeVal)) == -1)
		return -1;


	return 0;



	// val->sec = 0;
	// val->usec = 0;

	/* The code in `ch3` will leads to memory bugs*/

	// uint64 cycle = get_cycle();
	// val->sec = cycle / CPU_FREQ;
	// val->usec = (cycle % CPU_FREQ) * 1000000 / CPU_FREQ;
	//return 0;
}

uint64 sys_sbrk(int n)
{
	uint64 addr;
        struct proc *p = curr_proc();
        addr = p->program_brk;
        if(growproc(n) < 0)
                return -1;
        return addr;	
}



// TODO: add support for mmap and munmap syscall.
// hint: read through docstrings in vm.c. Watching CH4 video may also help.
// Note the return value and PTE flags (especially U,X,W,R)
/*
* LAB1: you may need to define sys_task_info here
*/


int sys_mmap(void* start, unsigned long long len, int port, int flag, int fd) {
        uint64 addr_start = (uint64) start;
        if (addr_start % PAGE_SIZE != 0) return -1;
        if (len > 1024 * 1024 * 1024) return -1;
        if ((port & (~0x7)) != 0) return -1;
        if ((port & 0x7) == 0) return -1;
        if (len == 0) return 0;
  		uint64 last = PGROUNDDOWN(addr_start + len - 1);
        char *mem;
        struct proc *p = curr_proc();
        for (uint64 i = addr_start; i <= last; i += PGSIZE) {
                mem = kalloc();
                if (mem == 0) return -1; // 这里没有考虑页回收
                memset(mem, 0, PGSIZE);
                if (mappages(p->pagetable, i, PGSIZE, (uint64)mem, (port<<1)|PTE_U) != 0) {
                        return -1;
                }
				// printf("%x %x\n", i, last);
        }
        return 0;
}









int sys_munmap(void* start, unsigned long long len) {
        struct proc *p = curr_proc();
        uint64 addr_start = (uint64)start;
        if (addr_start % PGSIZE != 0) return -1;
        uint64 last = PGROUNDDOWN(addr_start + len - 1);
        int npages = (last - addr_start + PGSIZE) / PGSIZE;
		if (uvmmunmap(p->pagetable, addr_start, npages) != 0){
			return -1;
		}
        return 0;
}



uint64 sys_task_info(TaskInfo *ti){

	struct proc *p = curr_proc();
	char src[sizeof(TaskInfo)];

	TaskInfo* info = (TaskInfo*)src;
	//status
	info->status = Running;
	//time
	uint64 cycle = get_cycle();
	uint64 start_time = curr_proc()->start_time;	
	info->time = (cycle / CPU_FREQ) * 1000 + (cycle % CPU_FREQ) * 1000 / CPU_FREQ - start_time;
	//syscall_times
	for (int i = 0; i < MAX_SYSCALL_NUM; i++) info->syscall_times[i] = curr_proc()->syscall_times[i];	

	if (copyout(p->pagetable, (uint64)ti, src, sizeof(TaskInfo)) == -1)
		return -1;

    return 0;
}




extern char trap_page[];

void syscall()
{
	struct trapframe *trapframe = curr_proc()->trapframe;
	int id = trapframe->a7, ret;
	uint64 args[6] = { trapframe->a0, trapframe->a1, trapframe->a2,
			   trapframe->a3, trapframe->a4, trapframe->a5 };
	tracef("syscall %d args = [%x, %x, %x, %x, %x, %x]", id, args[0],
	       args[1], args[2], args[3], args[4], args[5]);
	/*
	* LAB1: you may need to update syscall counter for task info here
	*/
	switch (id) {
	case SYS_write:
		curr_proc()->syscall_times[SYS_write] ++;
		ret = sys_write(args[0], args[1], args[2]);
		break;
	case SYS_exit:
		curr_proc()->syscall_times[SYS_exit] ++;
		sys_exit(args[0]);
		// __builtin_unreachable();
	case SYS_sched_yield:
		curr_proc()->syscall_times[SYS_sched_yield] ++;
		ret = sys_sched_yield();
		break;
	case SYS_gettimeofday:
		curr_proc()->syscall_times[SYS_gettimeofday] ++;
		ret = sys_gettimeofday((TimeVal *)args[0], args[1]);
		break;
	case SYS_sbrk:
		curr_proc()->syscall_times[SYS_sbrk] ++;
		ret = sys_sbrk(args[0]);
		break;
	/*
	* LAB1: you may need to add SYS_taskinfo case here
	*/

	case SYS_task_info:
		curr_proc()->syscall_times[SYS_task_info] ++;
		ret = sys_task_info((TaskInfo *)args[0]);
		break;

	case SYS_mmap:
		ret = sys_mmap((void *)args[0], (unsigned long long)args[1], (int)args[2], (int)args[3], (int)args[4]);
		break;

	case SYS_munmap:
		ret = sys_munmap((void *)args[0], (unsigned long long)args[1]);
		break;

	default:
		ret = -1;
		errorf("unknown syscall %d", id);
	}
	trapframe->a0 = ret;
	tracef("syscall ret %d", ret);
}
