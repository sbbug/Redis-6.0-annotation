robj数据类型:
     
     定义位置server.h
     typedef struct redisObject {
        /* 对象的数据类型。占4个bit。
        可能的取值有5种：OBJSTRING, OBJLIST, OBJSET, OBJZSET, OBJ_HASH，
        分别对应Redis对外暴露的5种数据结构
        */
        unsigned type;
        //对象的内部表示方式（也可以称为编码）。占4个bit
        unsigned encoding:4;
        unsigned lru:LRU_BITS; /* LRU time (relative to global lru_clock) or
                                * LFU data (least significant 8 bits frequency
                                * and most significant 16 bits access time). */
        int refcount;//引用计数，用来回收内存使用
        /*
        数据指针。指向真正的数据。
        比如，一个代表string的robj，它的ptr可能指向一个sds结构；
        一个代表list的robj，它的ptr可能指向一个quicklist
        */
        void *ptr;//该属性值可以被转换为其它五种类型
    } robj;
robj数据类型作用:
    作为通用数据结构，主要实现对string set list hash zset等类型的同一管理与定义。
    
Redis的embstr与raw编码方式:
    
    embstr编码存储的字符串长度较短时(len<=44 字节),Redis将会采用 embstr 编码.
    embstr 即embedded string 嵌入式的字符串.将SDS结构体嵌入RedisObject对象中, 
    使用 malloc 方法一次分配内存地址是连续的.
    
    当存储的字符串长度较长时(len>44 字节),Redis 将会采用 raw 编码,
    和 embstr 最大的区别就是 RedisObject 和 SDS 不在一起了,内存地址不再连续了.

