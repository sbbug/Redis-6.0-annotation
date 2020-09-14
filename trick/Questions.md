
#### Redis中key的过期机制如何实现？

###### 定时删除
       
       在进行set key value;expire key XXs时，可以创建一个定时器，到期执行定时器，删除过期key
       
###### 惰性删除

        每次在获取key时，先判断expires的key是否过期，通过key的属性when与now作比较，如果过期
        直接删除。

###### 定期删除

        每隔一段时间对数据库遍历检查，删除过期的key。

#### Redis的内存淘汰机制有哪些？


#### Redis如何实现内存的回收与释放

    Redis通过引用计数实现对象的回收与释放
    
    void incrRefCount(robj *o) {//引用计数++
        if (o->refcount < OBJ_FIRST_SPECIAL_REFCOUNT) {
            o->refcount++;
        } else {
            if (o->refcount == OBJ_SHARED_REFCOUNT) {
                /* Nothing to do: this refcount is immutable. */
            } else if (o->refcount == OBJ_STATIC_REFCOUNT) {
                serverPanic("You tried to retain an object allocated in the stack");
            }
        }
    }
    void decrRefCount(robj *o) {
        if (o->refcount == 1) {//如果引用树是1，释放内存
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
    //这里以String的回收作为例子
    void freeStringObject(robj *o) {
        if (o->encoding == OBJ_ENCODING_RAW) {
            sdsfree(o->ptr);
        }
    }
    void sdsfree(sds s) {
        if (s == NULL) return;
        s_free((char*)s-sdsHdrSize(s[-1]));
    }
    
    #define s_free zfree
    //内存释放  zfree位于内存管理模块下面
    void zfree(void *ptr) {
        #ifndef HAVE_MALLOC_SIZE
            void *realptr;
            size_t oldsize;
        #endif
        
            if (ptr == NULL) return;
        #ifdef HAVE_MALLOC_SIZE
            update_zmalloc_stat_free(zmalloc_size(ptr));
            free(ptr);//释放内存
        #else
            realptr = (char*)ptr-PREFIX_SIZE;//指针移动
            oldsize = *((size_t*)realptr);
            update_zmalloc_stat_free(oldsize+PREFIX_SIZE);
            free(realptr);//释放内存
        #endif
    }


