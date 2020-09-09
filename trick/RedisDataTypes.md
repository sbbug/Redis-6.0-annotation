
## Redis五种数据类型

    redis的五种数据类型分别是:string hash list set sorted set
    Redis五种数据类型是通过redisObject结构体作为中介接口提供给客户使用。
         由于Redis是一个key-value格式的数据库，并且key主要是字符串型，这里的五种数据类型
         主要是指value。
    redisObject代码：
        typedef struct redisObject {
            unsigned type:4;
            unsigned encoding:4;
            unsigned lru:LRU_BITS; /* LRU time (relative to global lru_clock) or
                                    * LFU data (least significant 8 bits frequency
                                    * and most significant 16 bits access time). */
            int refcount;
            void *ptr;//通过ptr指针指向对应结构
        } robj;
