/*
 * Taked from:
 * linux/ipc/util.h
 * Copyright (C) 1999 Christoph Rohland
 *
 * 2.16.2004: adapted to RTAI hard real time by:
 * Paolo Mantegazza (mantegazza@aero.polimi.it)
 */


#include <rtai_schedcore.h>
#include <rtai_sem.h>
#include "rtai_sysvmsg.h"

#define semaphore        rt_semaphore
#define sema_init(a, b)  rt_typed_sem_init(a, b, RES_SEM)
#define down(a)          rt_sem_wait(a)
#define up(a)            rt_sem_signal(a)

#if 1
#define schedule()          do { rt_task_suspend(0);                } while (0)
#define wake_up_process(t)  do { rt_task_resume(t->rtai_tskext[0]); } while (0)
#else
#define schedule()          do { int i; rt_receive(0, &i);      } while (0)
#define wake_up_process(t)  do { rt_send(t->rtai_tskext[0], 0); } while (0)
#endif

#define kmalloc(a, b)  rt_malloc(a)
#define kfree(a)       rt_free(a)

#define vmalloc(a)     rt_malloc(a)
#define vfree(a)       rt_free(a)

#undef spin_lock
#undef spin_unlock
#ifdef CONFIG_SMP
#define spin_lock(a)    rt_spin_lock_irq(a)
#define spin_unlock(a)  rt_spin_unlock_irq(a)
#else
#define spin_lock(a)
#define spin_unlock(a)
#endif

#define MODULE_NAME "RTAI_SYSVMSG"

#define USHRT_MAX  0xffff
#define SEQ_MULTIPLIER	(IPCMNI)

void msg_init (void);
void msg_exit (void);

struct ipc_ids {
	int size;
	int in_use;
	int max_id;
	unsigned short seq;
	unsigned short seq_max;
	struct semaphore sem;
	spinlock_t ary;
	struct ipc_id* entries;
};

struct ipc_id {
	struct kern_ipc_perm* p;
};

void __init ipc_init_ids(struct ipc_ids* ids, int size);

/* must be called with ids->sem acquired.*/
int ipc_findkey(struct ipc_ids* ids, key_t key);
int ipc_addid(struct ipc_ids* ids, struct kern_ipc_perm* new, int size);

/* must be called with both locks acquired. */
struct kern_ipc_perm* ipc_rmid(struct ipc_ids* ids, int id);

int ipcperms (struct kern_ipc_perm *ipcp, short flg);

extern inline void ipc_lockall(struct ipc_ids* ids)
{
	spin_lock(&ids->ary);
}

extern inline struct kern_ipc_perm* ipc_get(struct ipc_ids* ids, int id)
{
	struct kern_ipc_perm* out;
	int lid = id % SEQ_MULTIPLIER;
	if(lid >= ids->size)
		return NULL;

	out = ids->entries[lid].p;
	return out;
}

extern inline void ipc_unlockall(struct ipc_ids* ids)
{
	spin_unlock(&ids->ary);
}

extern inline struct kern_ipc_perm* ipc_lock(struct ipc_ids* ids, int id)
{
	struct kern_ipc_perm* out;
	int lid = id % SEQ_MULTIPLIER;
	if(lid >= ids->size)
		return NULL;

	spin_lock(&ids->ary);
	out = ids->entries[lid].p;
	if(out==NULL)
		spin_unlock(&ids->ary);
	return out;
}

extern inline void ipc_unlock(struct ipc_ids* ids, int id)
{
	spin_unlock(&ids->ary);
}

extern inline int ipc_buildid(struct ipc_ids* ids, int id, int seq)
{
	return SEQ_MULTIPLIER*seq + id;
}

extern inline int ipc_checkid(struct ipc_ids* ids, struct kern_ipc_perm* ipcp, int uid)
{
	if(uid/SEQ_MULTIPLIER != ipcp->seq)
		return 1;
	return 0;
}

void kernel_to_ipc64_perm(struct kern_ipc_perm *in, struct ipc64_perm *out);
void ipc64_perm_to_ipc_perm(struct ipc64_perm *in, struct ipc_perm *out);

int ipc_parse_version (int *cmd);
