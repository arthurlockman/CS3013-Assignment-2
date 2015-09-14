#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include "processinfo_sys.h"
#include <linux/sched.h>
#include <asm-generic/current.h>
#include <linux/time.h>
#include <asm-generic/cputime.h>
#include <linux/cred.h>

unsigned long **sys_call_table;

asmlinkage long (*ref_sys_cs3013_syscall2)(void);

asmlinkage long new_sys_cs3013_syscall2(struct processinfo *info) {
    struct task_struct *task_info = current;
    struct processinfo tmp;
    tmp.state = task_info->state;
    tmp.pid = task_info->pid;
    tmp.parent_pid = task_info->real_parent->pid;
    /* tmp.youngest_child = task_info->p_cptr->pid; */
    /* tmp.younger_sibling = task_info->p_ysptr->pid; */
    /* tmp.older_sibling = task_info->p_osptr->pid; */
    tmp.uid = task_info->real_cred->uid.val;
    tmp.start_time = timespec_to_ns(&task_info->start_time);
    tmp.user_time = cputime_to_usecs(&task_info->utime);
    tmp.sys_time = cputime_to_usecs(&task_info->stime);
    tmp.cutime = 0;
    tmp.cstime = 0;
    if (copy_to_user(info, &tmp, sizeof tmp))
            return EFAULT;
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
