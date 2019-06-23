```cpp
/*
 *  fs/eventpoll.c (Efficient event retrieval implementation)
 *  Copyright (C) 2001,...,2009  Davide Libenzi
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 */
/*
 * �������˽�epoll��ʵ��֮ǰ, �����˽��ں˵�3������.
 * 1. �ȴ����� waitqueue
 * ���Ǽ򵥽���һ�µȴ�����:
 * ����ͷ(wait_queue_head_t)��������Դ������,
 * ���г�Ա(wait_queue_t)��������Դ������,
 * ��ͷ����Դready��, �����ִ��ÿ����Աָ���Ļص�����,
 * ��֪ͨ������Դ�Ѿ�ready��, �ȴ����д��¾������˼.
 * 2. �ں˵�poll����
 * ��Poll��fd, ������ʵ����֧���ں˵�Poll����,
 * ����fd��ĳ���ַ��豸,�����Ǹ�socket, ������ʵ��
 * file_operations�е�poll����, ���Լ�������һ���ȴ�����ͷ.
 * ����poll fd��ĳ�����̱������һ���ȴ����г�Ա, ��ӵ�
 * fd�ĶԴ���������ȥ, ��ָ����Դreadyʱ�Ļص�����.
 * ��socket������, ��������ʵ��һ��poll����, ���Poll��
 * ������ѯ�Ĵ�������������õ�, �ú����б������poll_wait(),
 * poll_wait�Ὣ��������Ϊ�ȴ����г�Ա���뵽socket�ĵȴ�������ȥ.
 * ����socket����״̬�仯ʱ����ͨ������ͷ���֪ͨ���й������Ľ���.
 * ��һ��������������, ������벻����epoll�����
 * ��֪fd��״̬�����仯��.
 * 3. epollfd����Ҳ�Ǹ�fd, ����������Ҳ���Ա�epoll,
 * ���Բ²�һ�����ǲ��ǿ�������Ƕ��epoll��ȥ...
 *
 * epoll�����Ͼ���ʹ���������1,2�������.
 * �ɼ�epoll����û�и��ں�����ʲô�ر��ӻ��߸���ļ���,
 * ֻ���������й��ܵ��������, �ﵽ�˳���select��Ч��.
 */
/*
 * ��ص������ں�֪ʶ:
 * 1. fd����֪�����ļ�������, ���ں�̬, ��֮��Ӧ����struct file�ṹ,
 * ���Կ������ں�̬���ļ�������.
 * 2. spinlock, ������, ����Ҫ�ǳ�С��ʹ�õ���,
 * �����ǵ���spin_lock_irqsave()��ʱ��, �жϹر�, ���ᷢ�����̵���,
 * ����������Դ����CPUҲ�޷�����. ������Ǻ�ǿ����, ����ֻ����һЩ
 * �ǳ��������Ĳ���.
 * 3. ���ü������ں����Ƿǳ���Ҫ�ĸ���,
 * �ں˴������澭����Щrelease, free�ͷ���Դ�ĺ������������κ���,
 * ������Ϊ��Щ�����������ڶ�������ü������0ʱ������,
 * ��Ȼû�н�����ʹ������Щ����, ��ȻҲ����Ҫ����.
 * struct file �ǳ������ü�����.
 */
/* --- epoll��ص����ݽṹ --- */
/*
 * This structure is stored inside the "private_data" member of the file
 * structure and rapresent the main data sructure for the eventpoll
 * interface.
 */
/* ÿ����һ��epollfd, �ں˾ͻ����һ��eventpoll��֮��Ӧ, ����˵��
 * �ں�̬��epollfd. */
struct eventpoll {
    /* Protect the this structure access */
    spinlock_t lock;
    /*
     * This mutex is used to ensure that files are not removed
     * while epoll is using them. This is held during the event
     * collection loop, the file cleanup path, the epoll file exit
     * code and the ctl operations.
     */
    /* ���, �޸Ļ���ɾ������fd��ʱ��, �Լ�epoll_wait����, ���û��ռ�
     * ��������ʱ����������������, �������û��ռ���Է��ĵ��ڶ���߳�
     * ��ͬʱִ��epoll��صĲ���, �ں˼��Ѿ����˱���. */
    struct mutex mtx;
    /* Wait queue used by sys_epoll_wait() */
    /* ����epoll_wait()ʱ, ���Ǿ���"˯"��������ȴ�������... */
    wait_queue_head_t wq;
    /* Wait queue used by file->poll() */
    /* �������epollfd���±�poll��ʱ��... */
    wait_queue_head_t poll_wait;
    /* List of ready file descriptors */
    /* �����Ѿ�ready��epitem��������������� */
    struct list_head rdllist;
    /* RB tree root used to store monitored fd structs */
    /* ����Ҫ������epitem�������� */
    struct rb_root rbr;
    /*
        ����һ�����������������е�struct epitem��eventת�Ƶ��û��ռ�ʱ
     */
     * This is a single linked list that chains all the "struct epitem" that
     * happened while transfering ready events to userspace w/out
     * holding ->lock.
     */
    struct epitem *ovflist;
    /* The user that created the eventpoll descriptor */
    /* ���ﱣ����һЩ�û�����, ����fd�������������ֵ�ȵ� */
    struct user_struct *user;
};
/*
 * Each file descriptor added to the eventpoll interface will
 * have an entry of this type linked to the "rbr" RB tree.
 */
/* epitem ��ʾһ����������fd */
struct epitem {
    /* RB tree node used to link this structure to the eventpoll RB tree */
    /* rb_node, ��ʹ��epoll_ctl()��һ��fds���뵽ĳ��epollfdʱ, �ں˻����
     * һ����epitem��fds�Ƕ�Ӧ, ����������rb_tree����ʽ��֯����, tree��root
     * ������epollfd, Ҳ����struct eventpoll��.
     * ������ʹ��rb_tree��ԭ������Ϊ����߲���,�����Լ�ɾ�����ٶ�.
     * rb_tree������3������������O(lgN)��ʱ�临�Ӷ� */
    struct rb_node rbn;
    /* List header used to link this structure to the eventpoll ready list */
    /* ����ڵ�, �����Ѿ�ready��epitem���ᱻ����eventpoll��rdllist�� */
    struct list_head rdllink;
    /*
     * Works together "struct eventpoll"->ovflist in keeping the
     * single linked chain of items.
     */
    /* ����ڴ������ٽ���... */
    struct epitem *next;
    /* The file descriptor information this item refers to */
    /* epitem��Ӧ��fd��struct file */
    struct epoll_filefd ffd;
    /* Number of active wait queue attached to poll operations */
    int nwait;
    /* List containing poll wait queues */
    struct list_head pwqlist;
    /* The "container" of this item */
    /* ��ǰepitem�����ĸ�eventpoll */
    struct eventpoll *ep;
    /* List header used to link this item to the "struct file" items list */
    struct list_head fllink;
    /* The structure that describe the interested events and the source fd */
    /* ��ǰ��epitem��ϵ��Щevents, ��������ǵ���epoll_ctlʱ���û�̬���ݹ��� */
    struct epoll_event event;
};
struct epoll_filefd {
    struct file *file;
    int fd;
};
/* poll���õ��Ĺ���Wait structure used by the poll hooks */
struct eppoll_entry {
    /* List header used to link this structure to the "struct epitem" */
    struct list_head llink;
    /* The "base" pointer is set to the container "struct epitem" */
    struct epitem *base;
    /*
     * Wait queue item that will be linked to the target file wait
     * queue head.
     */
    wait_queue_t wait;
    /* The wait queue head that linked the "wait" wait queue item */
    wait_queue_head_t *whead;
};
/* Wrapper struct used by poll queueing */
struct ep_pqueue {
    poll_table pt;
    struct epitem *epi;
};
/* Used by the ep_send_events() function as callback private data */
struct ep_send_events_data {
    int maxevents;
    struct epoll_event __user *events;
};
 
/* --- ����ע�� --- */
/* ��û����, �����epoll_create()������, ����ɶҲ����ֱ�ӵ���epoll_create1��,
 * ������Ҳ���Է���, size���������ʵ��û���κ��ô���... */
SYSCALL_DEFINE1(epoll_create, int, size)
{
        if (size <= 0)
                return -EINVAL;
        return sys_epoll_create1(0);
}
/* �����������epoll_create��~~ */
SYSCALL_DEFINE1(epoll_create1, int, flags)
{
    int error;
    struct eventpoll *ep = NULL;//��������
    /* Check the EPOLL_* constant for consistency.  */
    /* ���ûɶ�ô�... */
    BUILD_BUG_ON(EPOLL_CLOEXEC != O_CLOEXEC);
    /* ����epoll����, ĿǰΨһ��Ч��FLAG����CLOEXEC */
    if (flags & ~EPOLL_CLOEXEC)
        return -EINVAL;
    /*
     * Create the internal data structure ("struct eventpoll").
     */
    /* ����һ��struct eventpoll, ����ͳ�ʼ��ϸ�������������~ */
    error = ep_alloc(&ep);
    if (error < 0)
        return error;
    /*
     * Creates all the items needed to setup an eventpoll file. That is,
     * a file structure and a free file descriptor.
     */
    /* �����Ǵ���һ������fd, ˵�����ͻ�����...������˵:
     * epollfd����������һ���������ļ���֮��Ӧ, �����ں���Ҫ����һ��
     * "����"���ļ�, ��Ϊ֮����������struct file�ṹ, ������������fd.
     * ����2�������ȽϹؼ�:
     * eventpoll_fops, fops����file operations, ���ǵ��������ļ�(�����������)���в���(�����)ʱ,
     * fops����ĺ���ָ��ָ�������Ĳ���ʵ��, ����C++�����麯��������ĸ���.
     * epollֻʵ����poll��release(����close)����, �����ļ�ϵͳ��������VFSȫȨ������.
     * ep, ep����struct epollevent, ������Ϊһ��˽�����ݱ�����struct file��privateָ������.
     * ��ʵ˵����, ����Ϊ����ͨ��fd�ҵ�struct file, ͨ��struct file���ҵ�eventpoll�ṹ.
     * �����һ��Linux���ַ��豸��������, ����Ӧ���Ǻܺ�����,
     * �Ƽ��Ķ� <Linux device driver 3rd>
     */
    error = anon_inode_getfd("[eventpoll]", &eventpoll_fops, ep,
                 O_RDWR | (flags & O_CLOEXEC));
    if (error < 0)
        ep_free(ep);
    return error;
}
/*
* ������epollfd��, ����������Ҫ���������fd��
* ����epoll_ctl
* epfd ����epollfd
* op ADD,MOD,DEL
* fd ��Ҫ������������
* event ���ǹ��ĵ�events
*/
SYSCALL_DEFINE4(epoll_ctl, int, epfd, int, op, int, fd,
        struct epoll_event __user *, event)
{
    int error;
    struct file *file, *tfile;
    struct eventpoll *ep;
    struct epitem *epi;
    struct epoll_event epds;
    error = -EFAULT;
    /*
     * �������Լ����û��ռ佫epoll_event�ṹcopy���ں˿ռ�.
     */
    if (ep_op_has_event(op) &&
        copy_from_user(&epds, event, sizeof(struct epoll_event)))
        goto error_return;
    /* Get the "struct file *" for the eventpoll file */
    /* ȡ��struct file�ṹ, epfd��Ȼ��������fd, ��ô�ں˿ռ�
     * �ͻ�����֮���ڵ�һ��struct file�ṹ
     * ����ṹ��epoll_create1()��, �ɺ���anon_inode_getfd()���� */
    error = -EBADF;
    file = fget(epfd);
    if (!file)
        goto error_return;
    /* Get the "struct file *" for the target file */
    /* ������Ҫ������fd, ����ȻҲ�и�struct file�ṹ, ����2����Ҫ�����Ŷ */
    tfile = fget(fd);
    if (!tfile)
        goto error_fput;
    /* The target file descriptor must support poll */
    error = -EPERM;
    /* ����������ļ���֧��poll, �Ǿ�û����.
     * ��֪��ʲô�����, �ļ��᲻֧��poll��?
     */
    if (!tfile->f_op || !tfile->f_op->poll)
        goto error_tgt_fput;
    /*
     * We have to check that the file structure underneath the file descriptor
     * the user passed to us _is_ an eventpoll file. And also we do not permit
     * adding an epoll file descriptor inside itself.
     */
    error = -EINVAL;
    /* epoll�����Լ������Լ�... */
    if (file == tfile || !is_file_epoll(file))
        goto error_tgt_fput;
    /*
     * At this point it is safe to assume that the "private_data" contains
     * our own data structure.
     */
    /* ȡ�����ǵ�eventpoll�ṹ, ������epoll_create1()�еķ��� */
    ep = file->private_data;
    /* �������Ĳ����п����޸����ݽṹ����, ��֮~ */
    mutex_lock(&ep->mtx);
    /*
     * Try to lookup the file inside our RB tree, Since we grabbed "mtx"
     * above, we can be sure to be able to use the item looked up by
     * ep_find() till we release the mutex.
     */
    /* ����ÿһ��������fd, �ں˶��з���һ��epitem�ṹ,
     * ��������Ҳ֪��, epoll�ǲ������ظ����fd��,
     * �����������Ȳ��Ҹ�fd�ǲ����Ѿ�������.
     * ep_find()��ʵ����RBTREE����, ��C++STL��map���һ����, O(logn)��ʱ�临�Ӷ�.
     */
    epi = ep_find(ep, tfile, fd);
    error = -EINVAL;
    switch (op) {
        /* �������ǹ������ */
    case EPOLL_CTL_ADD:
        if (!epi) {
            /* ֮ǰ��findû���ҵ���Ч��epitem, ֤���ǵ�һ�β���, ����!
             * �������ǿ���֪��, POLLERR��POLLHUP�¼��ں����ǻ���ĵ�
             * */
            epds.events |= POLLERR | POLLHUP;
            /* rbtree����, �����ep_insert()�ķ���
             * ��ʵ�Ҿ���������insert�Ļ�, ֮ǰ��findӦ��
             * �ǿ���ʡ����... */
            error = ep_insert(ep, &epds, tfile, fd);
        } else
            /* �ҵ���!? �ظ����! */
            error = -EEXIST;
        break;
        /* ɾ�����޸Ĳ������Ƚϼ� */
    case EPOLL_CTL_DEL:
        if (epi)
            error = ep_remove(ep, epi);
        else
            error = -ENOENT;
        break;
    case EPOLL_CTL_MOD:
        if (epi) {
            epds.events |= POLLERR | POLLHUP;
            error = ep_modify(ep, epi, &epds);
        } else
            error = -ENOENT;
        break;
    }
    mutex_unlock(&ep->mtx);
error_tgt_fput:
    fput(tfile);
error_fput:
    fput(file);
error_return:
    return error;
}
/* ����һ��eventpoll�ṹ */
static int ep_alloc(struct eventpoll **pep)
{
    int error;
    struct user_struct *user;
    struct eventpoll *ep;
    /* ��ȡ��ǰ�û���һЩ��Ϣ, �����ǲ���root��, ������fd��Ŀ�� */
    user = get_current_user();
    error = -ENOMEM;
    ep = kzalloc(sizeof(*ep), GFP_KERNEL);
    if (unlikely(!ep))
        goto free_uid;
    /* ��Щ���ǳ�ʼ���� */
    spin_lock_init(&ep->lock);
    mutex_init(&ep->mtx);
    init_waitqueue_head(&ep->wq);//��ʼ���Լ�˯�ڵĵȴ�����
    init_waitqueue_head(&ep->poll_wait);//��ʼ��
    INIT_LIST_HEAD(&ep->rdllist);//��ʼ����������
    ep->rbr = RB_ROOT;
    ep->ovflist = EP_UNACTIVE_PTR;
    ep->user = user;
    *pep = ep;
    return 0;
free_uid:
    free_uid(user);
    return error;
}
/*
 * Must be called with "mtx" held.
 */
/*
 * ep_insert()��epoll_ctl()�б�����, �����epollfd�������һ������fd�Ĺ���
 * tfile��fd���ں�̬��struct file�ṹ
 */
static int ep_insert(struct eventpoll *ep, struct epoll_event *event,
             struct file *tfile, int fd)
{
    int error, revents, pwake = 0;
    unsigned long flags;
    struct epitem *epi;
    struct ep_pqueue epq;
    /* �鿴�Ƿ�ﵽ��ǰ�û����������� */
    if (unlikely(atomic_read(&ep->user->epoll_watches) >=
             max_user_watches))
        return -ENOSPC;
    /* ��������slab�з���һ��epitem */
    if (!(epi = kmem_cache_alloc(epi_cache, GFP_KERNEL)))
        return -ENOMEM;
    /* Item initialization follow here ... */
    /* ��Щ������س�Ա�ĳ�ʼ��... */
    INIT_LIST_HEAD(&epi->rdllink);
    INIT_LIST_HEAD(&epi->fllink);
    INIT_LIST_HEAD(&epi->pwqlist);
    epi->ep = ep;
    /* ���ﱣ����������Ҫ�������ļ�fd������file�ṹ */
    ep_set_ffd(&epi->ffd, tfile, fd);
    epi->event = *event;
    epi->nwait = 0;
    /* ���ָ��ĳ�ֵ����NULLŶ... */
    epi->next = EP_UNACTIVE_PTR;
    /* Initialize the poll table using the queue callback */
    /* ��, ��������Ҫ���뵽poll�������� */
    epq.epi = epi;
    /* ��ʼ��һ��poll_table
     * ��ʵ����ָ������poll_wait(ע�ⲻ��epoll_wait!!!)ʱ�Ļص�����,�����ǹ�����Щevents,
     * ep_ptable_queue_proc()�������ǵĻص���, ��ֵ������event������ */
    init_poll_funcptr(&epq.pt, ep_ptable_queue_proc);
    /*
     * Attach the item to the poll hooks and get current event bits.
     * We can safely use the file* here because its usage count has
     * been increased by the caller of this function. Note that after
     * this operation completes, the poll callback can start hitting
     * the new item.
     */
    /* ��һ���ܹؼ�, Ҳ�Ƚ��Ѷ�, ��ȫ���ں˵�poll���Ƶ��µ�...
     * ����, f_op->poll()һ����˵ֻ�Ǹ�wrapper, �������������pollʵ��,
     * ��UDP��socket������, ������������ĵ�������: f_op->poll(), sock_poll(),
     * udp_poll(), datagram_poll(), sock_poll_wait(), �����õ���������ָ����
     * ep_ptable_queue_proc()����ص�����...(����ĵ���·��...).
     * �����һ��, ���ǵ�epitem�͸����socket����������, ������״̬�仯ʱ,
     * ��ͨ��ep_poll_callback()��֪ͨ.
     * ���, ������������ѯ��ǰ��fd�ǲ����Ѿ���ɶevent�Ѿ�ready��, �еĻ�
     * �Ὣevent����. */
    revents = tfile->f_op->poll(tfile, &epq.pt);
    /*
     * We have to check if something went wrong during the poll wait queue
     * install process. Namely an allocation for a wait queue failed due
     * high memory pressure.
     */
    error = -ENOMEM;
    if (epi->nwait < 0)
        goto error_unregister;
    /* Add the current item to the list of active epoll hook for this file */
    /* �������ÿ���ļ��Ὣ���м����Լ���epitem������ */
    spin_lock(&tfile->f_lock);
    list_add_tail(&epi->fllink, &tfile->f_ep_links);
    spin_unlock(&tfile->f_lock);
    /*
     * Add the current item to the RB tree. All RB tree operations are
     * protected by "mtx", and ep_insert() is called with "mtx" held.
     */
    /* ���㶨��, ��epitem���뵽��Ӧ��eventpoll��ȥ */
    ep_rbtree_insert(ep, epi);
    /* We have to drop the new item inside our item list to keep track of it */
    spin_lock_irqsave(&ep->lock, flags);
    /* If the file is already "ready" we drop it inside the ready list */
    /* ���������, ������Ǽ�����fd�Ѿ����¼�����, �Ǿ�Ҫ����һ�� */
    if ((revents & event->events) && !ep_is_linked(&epi->rdllink)) {
        /* ����ǰ��epitem���뵽ready list��ȥ */
        list_add_tail(&epi->rdllink, &ep->rdllist);
        /* Notify waiting tasks that events are available */
        /* ˭��epoll_wait, �ͻ�����... */
        if (waitqueue_active(&ep->wq))
            wake_up_locked(&ep->wq);
        /* ˭��epoll��ǰ��epollfd, Ҳ������... */
        if (waitqueue_active(&ep->poll_wait))
            pwake++;
    }
    spin_unlock_irqrestore(&ep->lock, flags);
    atomic_inc(&ep->user->epoll_watches);
    /* We have to call this outside the lock */
    if (pwake)
        ep_poll_safewake(&ep->poll_wait);
    return 0;
error_unregister:
    ep_unregister_pollwait(ep, epi);
    /*
     * We need to do this because an event could have been arrived on some
     * allocated wait queue. Note that we don't care about the ep->ovflist
     * list, since that is used/cleaned only inside a section bound by "mtx".
     * And ep_insert() is called with "mtx" held.
     */
    spin_lock_irqsave(&ep->lock, flags);
    if (ep_is_linked(&epi->rdllink))
        list_del_init(&epi->rdllink);
    spin_unlock_irqrestore(&ep->lock, flags);
    kmem_cache_free(epi_cache, epi);
    return error;
}
/*
 * This is the callback that is used to add our wait queue to the
 * target file wakeup lists.
 */
/*
 * �ú����ڵ���f_op->poll()ʱ�ᱻ����.
 * Ҳ����epoll����pollĳ��fdʱ, ������epitem��ָ����fd����������.
 * �����İ취����ʹ�õȴ�����(waitqueue)
 */
static void ep_ptable_queue_proc(struct file *file, wait_queue_head_t *whead,
                 poll_table *pt)
{
    struct epitem *epi = ep_item_from_epqueue(pt);
    struct eppoll_entry *pwq;
    if (epi->nwait >= 0 && (pwq = kmem_cache_alloc(pwq_cache, GFP_KERNEL))) {
        /* ��ʼ���ȴ�����, ָ��ep_poll_callbackΪ����ʱ�Ļص�����,
         * �����Ǽ�����fd����״̬�ı�ʱ, Ҳ���Ƕ���ͷ������ʱ,
         * ָ���Ļص��������ᱻ����. */
        init_waitqueue_func_entry(&pwq->wait, ep_poll_callback);
        pwq->whead = whead;
        pwq->base = epi;
        /* ���շ���ĵȴ����г�Ա���뵽ͷ��, ͷ����fd���е� */
        add_wait_queue(whead, &pwq->wait);
        list_add_tail(&pwq->llink, &epi->pwqlist);
        /* nwait��¼�˵�ǰepitem���뵽�˶��ٸ��ȴ�������,
         * ����Ϊ���ֵ���Ҳֻ����1... */
        epi->nwait++;
    } else {
        /* We have to signal that an error occurred */
        epi->nwait = -1;
    }
}
/*
 * This is the callback that is passed to the wait queue wakeup
 * machanism. It is called by the stored file descriptors when they
 * have events to report.
 */
/*
 * ����ǹؼ��ԵĻص�����, �����Ǽ�����fd����״̬�ı�ʱ, ���ᱻ����.
 * ����key������һ��unsigned long����ʹ��, Я������events.
 */
static int ep_poll_callback(wait_queue_t *wait, unsigned mode, int sync, void *key)
{
    int pwake = 0;
    unsigned long flags;
    struct epitem *epi = ep_item_from_wait(wait);//�ӵȴ����л�ȡepitem.��Ҫ֪���ĸ����̹��ص�����豸
    struct eventpoll *ep = epi->ep;//��ȡ
    spin_lock_irqsave(&ep->lock, flags);
    /*
     * If the event mask does not contain any poll(2) event, we consider the
     * descriptor to be disabled. This condition is likely the effect of the
     * EPOLLONESHOT bit that disables the descriptor when an event is received,
     * until the next EPOLL_CTL_MOD will be issued.
     */
    if (!(epi->event.events & ~EP_PRIVATE_BITS))
        goto out_unlock;
    /*
     * Check the events coming with the callback. At this stage, not
     * every device reports the events in the "key" parameter of the
     * callback. We need to be able to handle both cases here, hence the
     * test for "key" != NULL before the event match test.
     */
    /* û�����ǹ��ĵ�event... */
    if (key && !((unsigned long) key & epi->event.events))
        goto out_unlock;
    /*
     * If we are trasfering events to userspace, we can hold no locks
     * (because we're accessing user memory, and because of linux f_op->poll()
     * semantics). All the events that happens during that period of time are
     * chained in ep->ovflist and requeued later on.
     */
    /*
     * ���￴���������е�ѽ�, ��ʵ�ɵ�����Ƚϼ�:
     * �����callback�����õ�ͬʱ, epoll_wait()�Ѿ�������,
     * Ҳ����˵, �˿�Ӧ�ó����п����Ѿ���ѭ����ȡevents,
     * ���������, �ں˽��˿̷���event��epitem��һ������������
     * ������, ������Ӧ�ó���, Ҳ������, ��������һ��epoll_wait
     * ʱ���ظ��û�.
     */
    if (unlikely(ep->ovflist != EP_UNACTIVE_PTR)) {
        if (epi->next == EP_UNACTIVE_PTR) {
            epi->next = ep->ovflist;
            ep->ovflist = epi;
        }
        goto out_unlock;
    }
    /* If this file is already in the ready list we exit soon */
    /* ����ǰ��epitem����ready list */
    if (!ep_is_linked(&epi->rdllink))
        list_add_tail(&epi->rdllink, &ep->rdllist);
    /*
     * Wake up ( if active ) both the eventpoll wait list and the ->poll()
     * wait list.
     */
    /* ����epoll_wait... */
    if (waitqueue_active(&ep->wq))
        wake_up_locked(&ep->wq);
    /* ���epollfdҲ�ڱ�poll, �Ǿͻ��Ѷ�����������г�Ա. */
    if (waitqueue_active(&ep->poll_wait))
        pwake++;
out_unlock:
    spin_unlock_irqrestore(&ep->lock, flags);
    /* We have to call this outside the lock */
    if (pwake)
        ep_poll_safewake(&ep->poll_wait);
    return 1;
}
/*
 * Implement the event wait interface for the eventpoll file. It is the kernel
 * part of the user space epoll_wait(2).
 */
SYSCALL_DEFINE4(epoll_wait, int, epfd, struct epoll_event __user *, events,
        int, maxevents, int, timeout)
{
    int error;
    struct file *file;
    struct eventpoll *ep;
    /* The maximum number of event must be greater than zero */
    if (maxevents <= 0 || maxevents > EP_MAX_EVENTS)
        return -EINVAL;
    /* Verify that the area passed by the user is writeable */
    /* ����ط��б�Ҫ˵��һ��:
     * �ں˶�Ӧ�ó����ȡ�Ĳ�����"���Բ�����",
     * �����ں˸�Ӧ�ó���֮������ݽ�������copy, ������(Ҳʱ��Ҳ�ǲ���...)ָ������.
     * epoll_wait()��Ҫ�ں˷������ݸ��û��ռ�, �ڴ����û������ṩ,
     * �����ں˻���һЩ�ֶ�����֤��һ���ڴ�ռ��ǲ�����Ч��.
     */
    if (!access_ok(VERIFY_WRITE, events, maxevents * sizeof(struct epoll_event))) {
        error = -EFAULT;
        goto error_return;
    }
    /* Get the "struct file *" for the eventpoll file */
    error = -EBADF;
    /* ��ȡepollfd��struct file, epollfdҲ���ļ��� */
    file = fget(epfd);
    if (!file)
        goto error_return;
    /*
     * We have to check that the file structure underneath the fd
     * the user passed to us _is_ an eventpoll file.
     */
    error = -EINVAL;
    /* ���һ�����ǲ���һ��������epollfd... */
    if (!is_file_epoll(file))
        goto error_fput;
    /*
     * At this point it is safe to assume that the "private_data" contains
     * our own data structure.
     */
    /* ��ȡeventpoll�ṹ */
    ep = file->private_data;
    /* Time to fish for events ... */
    /* OK, ˯��, �ȴ��¼�����~~ */
    error = ep_poll(ep, events, maxevents, timeout);
error_fput:
    fput(file);
error_return:
    return error;
}
/* �������������ִ��epoll_wait�Ľ��̴���˯��״̬... */
static int ep_poll(struct eventpoll *ep, struct epoll_event __user *events,
           int maxevents, long timeout)
{
    int res, eavail;
    unsigned long flags;
    long jtimeout;
    wait_queue_t wait;//�ȴ�����
    /*
     * Calculate the timeout by checking for the "infinite" value (-1)
     * and the overflow condition. The passed timeout is in milliseconds,
     * that why (t * HZ) / 1000.
     */
    /* ����˯��ʱ��, ����Ҫת��ΪHZ */
    jtimeout = (timeout < 0 || timeout >= EP_MAX_MSTIMEO) ?
        MAX_SCHEDULE_TIMEOUT : (timeout * HZ + 999) / 1000;
retry:
    spin_lock_irqsave(&ep->lock, flags);
    res = 0;
    /* ���ready list��Ϊ��, �Ͳ�˯��, ֱ�Ӹɻ�... */
    if (list_empty(&ep->rdllist)) {
        /*
         * We don't have any available event to return to the caller.
         * We need to sleep here, and we will be wake up by
         * ep_poll_callback() when events will become available.
         */
        /* OK, ��ʼ��һ���ȴ�����, ׼��ֱ�Ӱ��Լ�����,
         * ע��current��һ����, ����ǰ���� */
        init_waitqueue_entry(&wait, current);//��ʼ���ȴ�����,wait��ʾ��ǰ����
        __add_wait_queue_exclusive(&ep->wq, &wait);//���ص�ep�ṹ�ĵȴ�����
        for (;;) {
            /*
             * We don't want to sleep if the ep_poll_callback() sends us
             * a wakeup in between. That's why we set the task state
             * to TASK_INTERRUPTIBLE before doing the checks.
             */
            /* ����ǰ��������λ˯��, ���ǿ��Ա��źŻ��ѵ�״̬,
             * ע�����������"����ʱ", ���Ǵ˿̻�û˯! */
            set_current_state(TASK_INTERRUPTIBLE);
            /* ������ʱ��, ready list�����г�Ա��,
             * ����˯��ʱ���Ѿ�����, ��ֱ�Ӳ�˯��... */
            if (!list_empty(&ep->rdllist) || !jtimeout)
                break;
            /* ������źŲ���, Ҳ��... */
            if (signal_pending(current)) {
                res = -EINTR;
                break;
            }
            /* ɶ�¶�û��,����, ˯��... */
            spin_unlock_irqrestore(&ep->lock, flags);
            /* jtimeout���ʱ���, �ᱻ����,
             * ep_poll_callback()�����ʱ������,
             * ��ô���Ǿͻ�ֱ�ӱ�����, ���õ�ʱ����...
             * �ٴ�ǿ��һ��ep_poll_callback()�ĵ���ʱ�����ɱ�������fd
             * �ľ���ʵ��, ����socket����ĳ���豸������������,
             * ��Ϊ�ȴ�����ͷ�����ǳ��е�, epoll�͵�ǰ����
             * ֻ�ǵ����ĵȴ�...
             **/
            jtimeout = schedule_timeout(jtimeout);//˯��
            spin_lock_irqsave(&ep->lock, flags);
        }
        __remove_wait_queue(&ep->wq, &wait);
        /* OK ����������... */
        set_current_state(TASK_RUNNING);
    }
    /* Is it worth to try to dig for events ? */
    eavail = !list_empty(&ep->rdllist) || ep->ovflist != EP_UNACTIVE_PTR;
    spin_unlock_irqrestore(&ep->lock, flags);
    /*
     * Try to transfer events to user space. In case we get 0 events and
     * there's still timeout left over, we go trying again in search of
     * more luck.
     */
    /* ���һ������, ��event����, �Ϳ�ʼ׼������copy���û��ռ���... */
    if (!res && eavail &&
        !(res = ep_send_events(ep, events, maxevents)) && jtimeout)
        goto retry;
    return res;
}
/* �����, ����ֱ����һ��... */
static int ep_send_events(struct eventpoll *ep,
              struct epoll_event __user *events, int maxevents)
{
    struct ep_send_events_data esed;
    esed.maxevents = maxevents;
    esed.events = events;
    return ep_scan_ready_list(ep, ep_send_events_proc, &esed);
}
/**
 * ep_scan_ready_list - Scans the ready list in a way that makes possible for
 *                      the scan code, to call f_op->poll(). Also allows for
 *                      O(NumReady) performance.
 *
 * @ep: Pointer to the epoll private data structure.
 * @sproc: Pointer to the scan callback.
 * @priv: Private opaque data passed to the @sproc callback.
 *
 * Returns: The same integer error code returned by the @sproc callback.
 */
static int ep_scan_ready_list(struct eventpoll *ep,
                  int (*sproc)(struct eventpoll *,
                       struct list_head *, void *),
                  void *priv)
{
    int error, pwake = 0;
    unsigned long flags;
    struct epitem *epi, *nepi;
    LIST_HEAD(txlist);
    /*
     * We need to lock this because we could be hit by
     * eventpoll_release_file() and epoll_ctl().
     */
    mutex_lock(&ep->mtx);
    /*
     * Steal the ready list, and re-init the original one to the
     * empty list. Also, set ep->ovflist to NULL so that events
     * happening while looping w/out locks, are not lost. We cannot
     * have the poll callback to queue directly on ep->rdllist,
     * because we want the "sproc" callback to be able to do it
     * in a lockless way.
     */
    spin_lock_irqsave(&ep->lock, flags);
    /* ��һ��Ҫע��, ����, ���м�����events��epitem������rdllist����,
     * ������һ��֮��, ���е�epitem��ת�Ƶ���txlist��, ��rdllist�������,
     * Ҫע��Ŷ, rdllist�Ѿ��������! */
    list_splice_init(&ep->rdllist, &txlist);
    /* ovflist, ��ep_poll_callback()�����ҽ��͹�, ��ʱ�˿����ǲ�ϣ��
     * ���µ�event���뵽ready list����, ������´��ٴ���... */
    ep->ovflist = NULL;
    spin_unlock_irqrestore(&ep->lock, flags);
    /*
     * Now call the callback function.
     */
    /* ������ص��������洦��ÿ��epitem
     * sproc ���� ep_send_events_proc, �����ע�͵�. */
    error = (*sproc)(ep, &txlist, priv);
    spin_lock_irqsave(&ep->lock, flags);
    /*
     * During the time we spent inside the "sproc" callback, some
     * other events might have been queued by the poll callback.
     * We re-insert them inside the main ready-list here.
     */
    /* ��������������ovflist, ��Щepitem���������ڴ������ݸ��û��ռ�ʱ
     * ���������¼�. */
    for (nepi = ep->ovflist; (epi = nepi) != NULL;
         nepi = epi->next, epi->next = EP_UNACTIVE_PTR) {
        /*
         * We need to check if the item is already in the list.
         * During the "sproc" callback execution time, items are
         * queued into ->ovflist but the "txlist" might already
         * contain them, and the list_splice() below takes care of them.
         */
        /* ����Щֱ�ӷ���readylist */
        if (!ep_is_linked(&epi->rdllink))
            list_add_tail(&epi->rdllink, &ep->rdllist);
    }
    /*
     * We need to set back ep->ovflist to EP_UNACTIVE_PTR, so that after
     * releasing the lock, events will be queued in the normal way inside
     * ep->rdllist.
     */
    ep->ovflist = EP_UNACTIVE_PTR;
    /*
     * Quickly re-inject items left on "txlist".
     */
    /* ��һ��û�д������epitem, ���²��뵽ready list */
    list_splice(&txlist, &ep->rdllist);
    /* ready list��Ϊ��, ֱ�ӻ���... */
    if (!list_empty(&ep->rdllist)) {
        /*
         * Wake up (if active) both the eventpoll wait list and
         * the ->poll() wait list (delayed after we release the lock).
         */
        if (waitqueue_active(&ep->wq))
            wake_up_locked(&ep->wq);
        if (waitqueue_active(&ep->poll_wait))
            pwake++;
    }
    spin_unlock_irqrestore(&ep->lock, flags);
    mutex_unlock(&ep->mtx);
    /* We have to call this outside the lock */
    if (pwake)
        ep_poll_safewake(&ep->poll_wait);
    return error;
}
/* �ú�����Ϊcallbakc��ep_scan_ready_list()�б�����
 * head��һ������, �������Ѿ�ready��epitem,
 * �������eventpoll�����ready list, �������溯���е�txlist.
 */
static int ep_send_events_proc(struct eventpoll *ep, struct list_head *head,
                   void *priv)
{
    struct ep_send_events_data *esed = priv;
    int eventcnt;
    unsigned int revents;
    struct epitem *epi;
    struct epoll_event __user *uevent;
    /*
     * We can loop without lock because we are passed a task private list.
     * Items cannot vanish during the loop because ep_scan_ready_list() is
     * holding "mtx" during this call.
     */
    /* ɨ����������... */
    for (eventcnt = 0, uevent = esed->events;
         !list_empty(head) && eventcnt < esed->maxevents;) {
        /* ȡ����һ����Ա */
        epi = list_first_entry(head, struct epitem, rdllink);
        /* Ȼ������������Ƴ� */
        list_del_init(&epi->rdllink);
        /* ��ȡevents,
         * ע��events����ep_poll_callback()�����Ѿ�ȡ��һ����, Ϊɶ��Ҫ��ȡ?
         * 1. ���ǵ�Ȼϣ�����õ��˿̵���������, events�ǻ���~
         * 2. �������е�pollʵ��, ��ͨ���ȴ����д�����events, �п���ĳЩ����ѹ��û��
         * ��������ȥ��ȡ. */
        revents = epi->ffd.file->f_op->poll(epi->ffd.file, NULL) &
            epi->event.events;
        if (revents) {
            /* ����ǰ���¼����û���������ݶ�copy���û��ռ�,
             * ����epoll_wait()��Ӧ�ó����ܶ�������һ������. */
            if (__put_user(revents, &uevent->events) ||
                __put_user(epi->event.data, &uevent->data)) {
                list_add(&epi->rdllink, head);
                return eventcnt ? eventcnt : -EFAULT;
            }
            eventcnt++;
            uevent++;
            if (epi->event.events & EPOLLONESHOT)
                epi->event.events &= EP_PRIVATE_BITS;
            else if (!(epi->event.events & EPOLLET)) {
                /* �ٺ�, EPOLLET�ͷ�ET�����������һ��֮��ѽ~
                 * �����ET, epitem�ǲ����ٽ��뵽readly list,
                 * ����fd�ٴη�����״̬�ı�, ep_poll_callback������.
                 * ����Ƿ�ET, �����㻹��û����Ч���¼���������,
                 * ���ᱻ���²��뵽ready list, ����һ��epoll_wait
                 * ʱ, ����������, ��֪ͨ���û��ռ�. ��Ȼ������
                 * ��������fdsȷʵû�¼�Ҳû������, epoll_wait�᷵��һ��0,
                 * ��תһ��.
                 */
                list_add_tail(&epi->rdllink, &ep->rdllist);
            }
        }
    }
    return eventcnt;
}
/* ep_free��epollfd��closeʱ����,
 * �ͷ�һЩ��Դ����, �Ƚϼ� */
static void ep_free(struct eventpoll *ep)
{
    struct rb_node *rbp;
    struct epitem *epi;
    /* We need to release all tasks waiting for these file */
    if (waitqueue_active(&ep->poll_wait))
        ep_poll_safewake(&ep->poll_wait);
    /*
     * We need to lock this because we could be hit by
     * eventpoll_release_file() while we're freeing the "struct eventpoll".
     * We do not need to hold "ep->mtx" here because the epoll file
     * is on the way to be removed and no one has references to it
     * anymore. The only hit might come from eventpoll_release_file() but
     * holding "epmutex" is sufficent here.
     */
    mutex_lock(&epmutex);
    /*
     * Walks through the whole tree by unregistering poll callbacks.
     */
    for (rbp = rb_first(&ep->rbr); rbp; rbp = rb_next(rbp)) {
        epi = rb_entry(rbp, struct epitem, rbn);
        ep_unregister_pollwait(ep, epi);
    }
    /*
     * Walks through the whole tree by freeing each "struct epitem". At this
     * point we are sure no poll callbacks will be lingering around, and also by
     * holding "epmutex" we can be sure that no file cleanup code will hit
     * us during this operation. So we can avoid the lock on "ep->lock".
     */
    /* ֮�����ڹر�epollfd֮ǰ����Ҫ����epoll_ctl�Ƴ��Ѿ���ӵ�fd,
     * ����Ϊ�����Ѿ�����... */
    while ((rbp = rb_first(&ep->rbr)) != NULL) {
        epi = rb_entry(rbp, struct epitem, rbn);
        ep_remove(ep, epi);
    }
    mutex_unlock(&epmutex);
    mutex_destroy(&ep->mtx);
    free_uid(ep->user);
    kfree(ep);
}
/* File callbacks that implement the eventpoll file behaviour */
static const struct file_operations eventpoll_fops = {
    .release    = ep_eventpoll_release,
    .poll       = ep_eventpoll_poll
};
/* Fast test to see if the file is an evenpoll file */
static inline int is_file_epoll(struct file *f)
{
    return f->f_op == &eventpoll_fops;
}
/* OK, eventpoll����Ϊ�Ƚ���Ҫ�ĺ�����ע������... */
```
