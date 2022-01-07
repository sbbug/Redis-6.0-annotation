
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

#### Redis的QPS于Mysql的QPS对比？
    
    Redis数据库QPS大约10w，MySQL数据库QPS大约5000。

#### master节点宕机，slave升级master，期间会影响其它请求吗
    客户端请求超时后，会有超时重试次数。
    具体参考 https://www.cnblogs.com/aquester/p/11428025.html。
    造成master节点宕机的才是根因。
    master节点宕机后，slave升级为master期间，可能会造成数据丢失。

#### 客户端网络连接生命周期
    首先通过TCP协议建立socket连接，服务端保存socket连接符，并监听连接是否有执行命令等请求。

#### client中reqtype字段含义
    Redis本身支持telnet请求与redis-cli请求，二者分别是PROTO_REQ_INLINE、PROTO_REQ_MULTIBULK。

#### Redis通信协议

    Redis 协议将传输的结构数据分为 5 种最小单元类型，单元结束时统一加上回车换行符号\r\n。
    1、单行字符 以 + 符号开头。
    2、多行字符 以 $ 符号开头，后跟字符长度。
    3、整数值 以 : 符号开头，后跟整数的字符串形式。
    4、错误消息 以 - 符号开头。
    5、数组 以 * 号开头，后跟数组的长度。   
     
    "*2\r\n$3\r\nget\r\n$4\r\nname\r\n"
     *2代表数组有两个元素，分别是get和name,\r\n该表示单元结束
     $3代表后面又三个字符，为get $4代表后面有四个字符，为name。
   参考
        
     https://www.cnblogs.com/wuwuyong/p/11757845.html