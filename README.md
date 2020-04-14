# FSAC KERNEL (Framework for Scheduling Algorithm Creation)

**Author: Carlos Bilbao Muñoz**		

GitHub: https://github.com/Zildj1an                       					


| #  | File created                 | Purpose                                                                                   |
|----|------------------------------|-------------------------------------------------------------------------------------------|
| 1  | /fsac/fsac.c                 | Helps with the management of the special linked lists, and the copy from user.            |
| 2  | /fsac/fsac_list.c            | Helps with the management of the special linked lists, and the copy from user.            |
| 3  | /fsac/fsac_plugin.c          | Dummy FSAC plugin (default) also manages the list of registered plugins.                  |
| 4  | /fsac/fsac_proc.c            | Manages the /proc entries. Check loaded plugins, check/change active plugin, stats on it. |
| 5  | /include/fsac/fsac.h         | Header of /fsac/fsac.c. Also includes auxiliary definitions/functions.                    |
| 6  | /include/fsac/fsac_list.h    | Header of /fsac/fsac_list.c. Also includes auxiliary function.                            |
| 7  | /include/fsac/fsac_macros.h  | Some macros of FSAC.                                                                      |
| 8  | /include/fsac/fsac_proc.h    | Header of /fsac/fsac_proc.c.                                                              |
| 9  | /kernel/sched/fsac_class.c   | FSAC new scheduling class, will mostly always delegate on the plugin.                     |
| 10 | /include/fsac/fsac_preempt.h | Header of /fsac/fsac_preempt.c                                                            |
| 11 | /fsac/fsac_preempt.c         | Functions to switch and check the preemption states, for rescheduling.                    |
| 12 | /include/fsac/fsac_param.h   | The extra parameter added to the struct task struct                                       |
| 13 | /include/fsac/fsac_np.h      | Postponed to future versions, includes functions to manage non-preemptive sections.       |
| 14 | /fsac/Makefile               | Kbuild Makefile for compilation        |


| #  | File modified                | Main reason                                                                                                                                                                                                                                                                                                                                        |
|----|------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 1  | /kernel/sched/sched.h        | Helps with the management of the special linked lists, and the copy from user.                                                                                                                                                                                                                                                                     |
| 2  | /include/uapi/linux/sched.h  | Helps with the management of the special linked lists, and the copy from user.                                                                                                                                                                                                                                                                     |
| 3  | /kernel/sched.c              | Dummy FSAC plugin (default) also manages the list of registered plugins.                                                                                                                                                                                                                                                                           |
| 4  | /fs/select.c                 | Manages the /proc entries. Check loaded plugins, check/change active plugin, stats on it.                                                                                                                                                                                                                                                          |
| 5  | /kernel/locking/mutex.c      | Header of /fsac/fsac.c. Also includes auxiliary definitions/functions.                                                                                                                                                                                                                                                                             |
| 6  | /kernel/locking/rwsem-xadd.c | Same idea as /kernel/locking/mutex.c but with semaphores.                                                                                                                                                                                                                                                                                          |
| 7  | /kernel/time/hrtimer.c       | Same idea as /kernel/locking/mutex.c but for the kernel timer.                                                                                                                                                                                                                                                                                     |
| 8  | /mm/page-writeback.c         | FSAC RT tasks should get special treatment when it comes to writing back dirty pages at the address-space.                                                                                                                                                                                                                                         |
| 9  | /mm/page_alloc.c             | Special amendments at page allocation when the FSAC plugin is intended to be real-time.                                                                                                                                                                                                                                                            |
| 10 | /kernel/sched/core.c         | Make sure no Linux balancing, integrate FSAC scheduling class. Also, add hooks for the preemption state machine, like sched_state_entered_schedule() at __schedule(), which flags WILL_SCHEDULE. Per-cpu bool fsac_preemption_in_progress updated in   __schedule(). Also in fsac_fork I add if (is_fsac(p)){ p->sched_class = &fsac_sched_class;} |
| 11 | /include/linux/sched.h       | Add handling of Inter-Processor Interrupt (IPI) for FSAC, with sched_state_ipi() at case IPI_RESCHEDULE.                                                                                                                                                                                                                                           |
| 12 | /arch/arm/kernel/smp.c       | The IPI smp_reschedule_interrupt() might produce a transition in the FSAC sched_state machine.                                                                                                                                                                                                                                                     |
| 13 | /arch/x86/kernel/smp.c       | Same idea as for /arch/x86/kernel/smp.c                                                                                                                                                                                                                                                                                                            |
| 14 | /fs/exec.c                   | Added a hook for fsac_exec at do_execveat_common()                                                                                                                                                                                                                                                                                                 |
| 15 | /kernel/fork.c               | Added a hook for exit_fsac at __put_task_struct()        |
| 16 | /Makefile               | Added to top Makefile the fsac directory at core-y        |
