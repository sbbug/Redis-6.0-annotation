字典中的node节点:
     
     typedef struct dictEntry {//定义字典所用到的k-v结构体
        void *key;
        union {//value可以选择四种类型存储
            void *val;
            uint64_t u64;
            int64_t s64;
            double d;
        } v;
        struct dictEntry *next;//指向下一个节点
    } dictEntry;
字典中的公共方法:
     
    typedef struct dictType {//字典操作的公共方法
        uint64_t (*hashFunction)(const void *key);//根据关键字计算哈希值
        void *(*keyDup)(void *privdata, const void *key);//复制key
        void *(*valDup)(void *privdata, const void *obj);//复制value
        int (*keyCompare)(void *privdata, const void *key1, const void *key2);//对两个key进行比较
        void (*keyDestructor)(void *privdata, void *key);//销毁key
        void (*valDestructor)(void *privdata, void *obj);//销毁value
    } dictType;
    
字典中的table数据结构:

    typedef struct dictht {//用于存储table的数据结构
        dictEntry **table;//指向指针的指针
        unsigned long size;//大小
        unsigned long sizemask;
        unsigned long used;
    } dictht;
    
字典数据结构:

    typedef struct dict {
        dictType *type;//指向公共方法区
        void *privdata;//
        dictht ht[2];//定义两个table
        long rehashidx;//是否正在进行rehash,如果rehashidx==-1,没有正在rehash
        unsigned long iterators; //当前字典的迭代器数量
    } dict;
    
字典迭代器结构:

    typedef struct dictIterator {
        dict *d;//指向字典对象
        long index;
        int table, safe;//safe标记该迭代器是否线程安全
        dictEntry *entry, *nextEntry;//当前所指的节点，下一个节点
        /* unsafe iterator fingerprint for misuse detection. */
        long long fingerprint;
    } dictIterator;