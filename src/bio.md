
## bio模块

       bio模块主要用来在后台执行相关的任务。
       bio后台主要有三种任务类型：
       //后台有三种类型作业
        #define BIO_CLOSE_FILE    0 //关闭文件
        #define BIO_AOF_FSYNC     1 //AOF缓存异步存储
        #define BIO_LAZY_FREE     2 //懒汉式释放内存
        #define BIO_NUM_OPS       3 //一共三种类型的任务
       
       关闭文件描述符很耗时间吗，为什么需要异步执行？
       close()方法属于系统调用函数，系统调用函数非常耗时，因此最好采用异步的方式关闭
       文件描述符。

## Redis6.0新特性引入多线程
    
    //引入多线程、锁、条件变量
    static pthread_t bio_threads[BIO_NUM_OPS];//定义线程数组，初始值是3
    static pthread_mutex_t bio_mutex[BIO_NUM_OPS];//定义每个线程对应的锁对象
    static pthread_cond_t bio_newjob_cond[BIO_NUM_OPS];//条件变量
    static pthread_cond_t bio_step_cond[BIO_NUM_OPS];
    //指针数组，每个元素存储的是每种类型作业的头结点
    static list *bio_jobs[BIO_NUM_OPS]; //定义list* 类型的数组，三种类型的作业分别对应的队列
    
    //定义挂起的任务数量，一共三种任务，每种任务的挂起数量存储在long long数组中
    static unsigned long long bio_pending[BIO_NUM_OPS];
    //Redis单个线程栈大小是4MB
    #define REDIS_THREAD_STACK_SIZE (1024*1024*4)//Redis单个线程栈的大小

## 后台作业结构特点
   
       struct bio_job {//定义后台作业结构体
            time_t time; /* 作业创建时间 */         
            void *arg1, *arg2, *arg3;//定义三个参数指针
        };
        
        //后台作业执行接口
        void *bioProcessBackgroundJobs(void *arg);//处理后台作业
        void lazyfreeFreeObjectFromBioThread(robj *o);//对象释放
        void lazyfreeFreeDatabaseFromBioThread(dict *ht1, dict *ht2);//数据库释放
        void lazyfreeFreeSlotsMapFromBioThread(zskiplist *sl);//列表释放

## 后台任务执行过程

    pthread_mutex_lock(&bio_mutex[type]);//尝试获取对应类型的互斥锁
    //处理任务队列任务，核心代码
    while(1) {
        listNode *ln;

        //如果任务队列等于0，阻塞当前线程，放入条件队列，等待被唤醒
        if (listLength(bio_jobs[type]) == 0) {
            pthread_cond_wait(&bio_newjob_cond[type],&bio_mutex[type]);
            continue;
        }
        //获取队列第一个任务,此时只是获取，并没有出队
        ln = listFirst(bio_jobs[type]);
        job = ln->value;
        /* 释放当前任务类型对应的锁，让任务队列接收任务入队请求.*/
        pthread_mutex_unlock(&bio_mutex[type]);//锁要及时释放

        /* 根据任务类型处理相关任务. */
        if (type == BIO_CLOSE_FILE) {
            close((long)job->arg1);//关闭文件描述符
        } else if (type == BIO_AOF_FSYNC) {
            redis_fsync((long)job->arg1);//异步落盘AOF文件
        } else if (type == BIO_LAZY_FREE) {//懒汉式释放内存
            /* What we free changes depending on what arguments are set:
             * arg1 -> free the object at pointer.
             * arg2 & arg3 -> free two dictionaries (a Redis DB).
             * only arg3 -> free the skiplist. */
            if (job->arg1)
                lazyfreeFreeObjectFromBioThread(job->arg1);
            else if (job->arg2 && job->arg3)
                lazyfreeFreeDatabaseFromBioThread(job->arg2,job->arg3);
            else if (job->arg3)
                lazyfreeFreeSlotsMapFromBioThread(job->arg3);
        } else {
            serverPanic("Wrong job type in bioProcessBackgroundJobs().");
        }
        zfree(job);

        /* Lock again before reiterating the loop, if there are no longer
         * jobs to process we'll block again in pthread_cond_wait(). */
        pthread_mutex_lock(&bio_mutex[type]);//获取对应任务类型的锁
        listDelNode(bio_jobs[type],ln);//删除已经处理完成的任务
        bio_pending[type]--;//任务数量--

        /* Unblock threads blocked on bioWaitStepOfType() if any. */
        pthread_cond_broadcast(&bio_step_cond[type]);
    }
    /*
    上述代码中，作者先采取加锁方式，获取任务队列的队首任务。然后释放对应的锁。
    再然后对队首任务执行相关操作，当完成队首任务后，需要对任务队列加锁，删除任务。
    作者为了提高任务队列的使用频率，只是在获取任务和删除任务分别进行了加锁。
    这里其实有个矛盾点：这段有两次加锁与释放锁的过程，加锁和释放锁其实也是消耗资源的。
    但是通过两次加锁与释放锁可以防止其他线程阻塞
    */
     
## lazyfreeFreeObjectFromBioThread方法   
       
       //单个对象的删除
       void lazyfreeFreeObjectFromBioThread(robj *o) {//删除robj对象
            decrRefCount(o);
            atomicDecr(lazyfree_objects,1);
       }
       void decrRefCount(robj *o) {//对象内部的引用计数-1
            if (o->refcount == 1) {//如果引用数是1，释放内存
                //根据不同的数据类型执行不同的方法释放内存
                switch(o->type) {
                case OBJ_STRING: freeStringObject(o); break;
                case OBJ_LIST: freeListObject(o); break;
                case OBJ_SET: freeSetObject(o); break;
                case OBJ_ZSET: freeZsetObject(o); break;
                case OBJ_HASH: freeHashObject(o); break;
                case OBJ_MODULE: freeModuleObject(o); break;
                case OBJ_STREAM: freeStreamObject(o); break;
                default: serverPanic("Unknown object type"); break;
                }
                zfree(o);
            } else {
                if (o->refcount <= 0) serverPanic("decrRefCount against refcount <= 0");
                if (o->refcount != OBJ_SHARED_REFCOUNT) o->refcount--;
            }
        }
        
        //删除数据库对象
        void lazyfreeFreeDatabaseFromBioThread(dict *ht1, dict *ht2) {
            size_t numkeys = dictSize(ht1);
            dictRelease(ht1);//释放字典1
            dictRelease(ht2);//释放字典2
            atomicDecr(lazyfree_objects,numkeys);
        }
        
## 线程相关方法
    
    1、
        pthread_mutex_lock(pthread_mutex_t *mutex);//调用该方法的线程尝试获取mutex锁，如果获取失败则阻塞等待。
        pthread_mutex_lock()函数锁住由mutex指定的mutex 对象。如果mutex已经被锁住，调用这个函数的线程阻塞直到mutex可用为止。
        这跟函数返回的时候参数mutex指定的mutex对象变成锁住状态，同时该函数的调用线程成为该mutex对象的拥有者。
        
    2、
        int pthread_mutex_unlock(pthread_mutex_t *mutex);//调用该方法实现对mutex锁的释放
        
    3、
        条件变量是利用线程间共享的全局变量进行同步的一种机制，主要包括两个动作：一个线程等待"条件变量的条件成立"而挂起；
        另一个线程使"条件成立"（给出条件成立信号）。为了防止竞争，条件变量的使用总是和一个互斥锁结合在一起。
        条件变量和操作系统中的管程、Java中lock与condition等非常相似。
        pthread_cond_wait()//实现当前线程的阻塞
        pthread_cond_signal()//实现线程的唤醒，与pthread_cond_signal相对应。
    4、
        pthread_cond_broadcast()方法
    