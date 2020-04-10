=============================================================
= **FSAC KERNEL (Framework for Scheduling Algorithm Creation)** =
= **author: Carlos Bilbao Muñoz**								=
= GitHub                        							=
=============================================================

See http://kernel.org/

File created:		              Purpose:
-------------                     -----------
1 /fsac/fsac.c		              Initializes everything: the proc and registers the dummy plugin. 
2 /fsac/fsac_list.c	              Helps with the management of the special linked lists, and the copy from user.	
3 /fsac/fsac_plugin.c	          Dummy FSAC plugin (default) also manages the list of registered plugins.
4 /fsac/fsac_proc.c	              Manages the /proc entries. Check loaded plugins, check/change active plugin, stats on it.
5 /include/fsac/fsac.h            Header of /fsac/fsac.c. Also includes auxiliary definitions/functions.
6 /include/fsac/fsac_list.h       Header of /fsac/fsac_list.c. Also includes auxiliary function.
7 /include/fsac/fsac_macros.h     Some macros of FSAC.
8 /include/fsac/fsac_proc.h       Header of /fsac/fsac_proc.c.
9 /kernel/sched/fsac_class.c      FSAC new scheduling class, will mostly always delegate on the plugin.
10 /include/fsac/fsac_preempt.h        
11 /fsac/fsac_preempt.c           Functions to switch and check the preemption states, for rescheduling.
12 /include/fsac/fsac_param.h     The extra parameter added to the struct task struct 
13 /include/fsac/fsac_np.h        Postponed to future versions, includes functions to manage non-preemptive sections.

File modified:		              Why:
--------------                    -----------
10 /kernel/sched/sched.h		  Made FSAC class a valid policy, add it to the linked list as highest. 
11 /include/uapi/linux/sched.h    Add macro SCHED_FSAC from /include/fsac/fsac_macros.h.
12 /kernel/sched.c                Hook to fsac_do_exit() from fsac.h when do_exit is called (ln. 729)
13 /fs/select.c                   Needed to avoid delays if the FSAC plugin is real-time.
14 /kernel/locking/mutex.c        Needed to avoid live-lock if the FSAC plugin is real-time.
15 /kernel/locking/rwsem-xadd.c   Same idea as /kernel/locking/mutex.c but with semaphores.
16 /kernel/time/hrtimer.c         Same idea as /kernel/locking/mutex.c but for the kernel timer.
17 /mm/page-writeback.c           FSAC RT tasks should get special treatment when it comes to writing back dirty pages at the address-space.
18 /mm/page_alloc.c               Special amendments at page allocation when the FSAC plugin is intended to be real-time.
19 /kernel/sched/core.c           Make sure no Linux balancing, integrate FSAC scheduling class. Also, add hooks for the preemption state machine, like
								  sched_state_entered_schedule() at __schedule(), which flags WILL_SCHEDULE. Per-cpu bool fsac_preemption_in_progress updated in 
								  __schedule(). Also in fsac_fork I add if (is_fsac(p)){ p->sched_class = &fsac_sched_class;}
20 /include/linux/sched.h         Add to struct task_struct the parameter fsac_param from /include/fsac/fsac_param.h. Also, add resched hook for the state machine.
21 /arch/arm/kernel/smp.c         Add handling of Inter-Processor Interrupt (IPI) for FSAC, with sched_state_ipi() at case IPI_RESCHEDULE.
22 /arch/x86/kernel/smp.c         The IPI smp_reschedule_interrupt() might produce a transition in the FSAC sched_state machine.
23 /arch/x86/xen/smp.c            Same idea as for /arch/x86/kernel/smp.c
24 /fs/exec.c                     Added a hook for fsac_exec at do_execveat_common()
25 /kernel/fork.c                 Added a hook for exit_fsac at __put_task_struct()

