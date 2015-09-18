/**
 * CS 3013 Project 2
 * Arthur Lockman and Tucker Haydon
 * part2.c
 *
 * This part of the projects intercepts and replaces our cs3013_syscall_1 
 * function that we compiled into the kernel with one that gets the 
 * task information for the currently running process.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include "processinfo_sys.h"
#include <linux/sched.h>
#include <asm-generic/current.h>
#include <linux/time.h>
#include <asm-generic/cputime.h>
#include <linux/cred.h>
#include <linux/list.h>

unsigned long **sys_call_table;

asmlinkage long (*ref_sys_cs3013_syscall2)(void);

asmlinkage long new_sys_cs3013_syscall2(struct processinfo *info) {
    struct task_struct *task_info = current; //initialize temporary data holders for various structs
    struct processinfo tmp;
    struct task_struct *youngest_child;
    struct task_struct *younger_sibling;
    struct task_struct *older_sibling;
    struct list_head *p;
    tmp.state = task_info->state; //Capture process state from task_info
    tmp.pid = task_info->pid; //Capture current PID
    tmp.parent_pid = task_info->parent->pid; //Capture parent PID

    if (!list_empty(&task_info->children)) //Check if there are any children, if there are find the youngest and store its PID
    {
        youngest_child = list_last_entry(&task_info->children, struct task_struct, children);
        tmp.youngest_child = youngest_child->pid;
    }
    else
    {
        tmp.youngest_child = -1; //If no children, set child PID to -1.
    }
    if (list_entry(task_info->sibling.next, struct task_struct, sibling)->pid > tmp.pid) //Check if there are any siblings
    {
        younger_sibling = list_entry(task_info->sibling.next, struct task_struct, sibling); //Store younger sibling if it exists
        tmp.younger_sibling = younger_sibling->pid;
    }
    else
    {
        tmp.younger_sibling = -1; //Set pid to -1 if there are no younger siblings.
    }
    if (list_entry(task_info->sibling.prev, struct task_struct, sibling)->pid < tmp.pid) //Check if there are any older siblings
    {
        older_sibling = list_entry(task_info->sibling.prev, struct task_struct, sibling); //Store PID if one exists
        tmp.older_sibling = older_sibling->pid;
    }
    else
    {
        tmp.older_sibling = -1; //Set PID to -1 if there are none.
    }

    tmp.uid = task_info->real_cred->uid.val; //Store UID extracted from real_cred struct in task_info.
    tmp.start_time = timespec_to_ns(&task_info->start_time); ///Capture and convert start time
    tmp.user_time = cputime_to_usecs(&task_info->utime); //Capture and convert user time
    tmp.sys_time = cputime_to_usecs(&task_info->stime); //Capture and convert system time
    tmp.cutime = 0; //initialize cutime and cstime variables
    tmp.cstime = 0;
    if (!list_empty(&task_info->children)) //Check if there are any children
    {
        list_for_each(p, &task_info->children) //If there are children, loop through each child
        {
            struct task_struct *s;
            s = list_entry(p, struct task_struct, children);
            tmp.cutime += cputime_to_usecs(&s->utime); //Accumulate child system and child user time
            tmp.cstime += cputime_to_usecs(&s->stime);
        }
    }

    if (copy_to_user(info, &tmp, sizeof tmp))
        return EFAULT; //Copy struct back to user.
    return 0;
}

static unsigned long **find_sys_call_table(void) {
    unsigned long int offset = PAGE_OFFSET;
    unsigned long **sct;

    while (offset < ULLONG_MAX) {
        sct = (unsigned long **)offset;

        if (sct[__NR_close] == (unsigned long *) sys_close) {
            printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX",
                    (unsigned long) sct);
            return sct;
        }

        offset += sizeof(void *);
    }

    return NULL;
}

static void disable_page_protection(void) {
    /*
       Control Register 0 (cr0) governs how the CPU operates.

       Bit #16, if set, prevents the CPU from writing to memory marked as
       read only. Well, our system call table meets that description.
       But, we can simply turn off this bit in cr0 to allow us to make
       changes. We read in the current value of the register (32 or 64
       bits wide), and AND that with a value where all bits are 0 except
       the 16th bit (using a negation operation), causing the write_cr0
       value to have the 16th bit cleared (with all other bits staying
       the same. We will thus be able to write to the protected memory.

       It's good to be the kernel!
       */
    write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
    /*
       See the above description for cr0. Here, we use an OR to set the 
       16th bit to re-enable write protection on the CPU.
       */
    write_cr0 (read_cr0 () | 0x10000);
}

static int __init interceptor_start(void) {
    /* Find the system call table */
    if(!(sys_call_table = find_sys_call_table())) {
        /* Well, that didn't work. 
           Cancel the module loading step. */
        return -1;
    }

    /* Store a copy of all the existing functions */
    ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];

    /* Replace the existing system calls */
    disable_page_protection();

    sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_sys_cs3013_syscall2;

    enable_page_protection();

    /* And indicate the load was successful */
    printk(KERN_INFO "Loaded interceptor!");

    return 0;
}

static void __exit interceptor_end(void) {
    /* If we don't know what the syscall table is, don't bother. */
    if(!sys_call_table)
        return;

    /* Revert all system calls to what they were before we began. */
    disable_page_protection();
    sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;
    enable_page_protection();

    printk(KERN_INFO "Unloaded interceptor!");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);
