
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
        
        void*指针可以指向任意类型的地址
        
## Redis字典的key-value数据结构

    typedef struct dictEntry {//定义字典所用到的k-v结构体
        void *key;
        union {
            void *val;
            uint64_t u64;
            int64_t s64;
            double d;
        } v;
        struct dictEntry *next;//指向下一个节点
    } dictEntry;
    union联合结构体的好处：
 
## 结构体内存对齐问题

#### 内存对齐原则

    1、数据成员对齐规则：结构（struct或联合union）的数据成员，第一个数据成员放在offset为0的地方，
        以后每个数据成员存储的起始位置要从该成员大小的整数倍开始（比如int在32位机为4字节，则要从4的整数倍地址开始存储）。
    2、结构体作为成员：如果一个结构里有某些结构体成员，则结构体成员要从其内部最大元素大小的整数倍地址开始存储。
     （struct a里存有struct b，b里有char，int，double等元素，那b应该从8的整数倍开始存储。）
    3、收尾工作：结构体的总大小，也就是sizeof的结果，必须是其内部最大成员的整数倍，不足的要补齐。
    
    例子:
    struct A{    
        int a;
        char b;
        short c;
    };
    struct B{
        char b;
        int a;
        short c;
    };
    sizeof(A) = 8B;
    sizeof(B) = 12B;
    
    为什么要有结构体数据对齐这个说法呢？
    这样从计算机底层原理说起，计算机在从存储器读取数据时，需要使用控制总线、地址总线、数据总线。
    一般来说地址总线和数据总线都是32位。CPU每次从PC指令寄存器获取下一条将要执行的指令的地址，然后
    通过地址总线到存储器中寻址，寻址完成后，通过数据总线将寻址到的数据传送到CPU内。
    由于数据总线的个数是32位，也就是一次可以传送四个字节的数据。CPU每次都会加载四个字节的数据。
    因此为了减少频繁的访问主存，CPU每个会读取4个字节数据到主存。
    
    采用字节对齐方式可以提升CPU的寻找速度，每次只需要偏移4个字节的地址，即可快速完成寻址。
    比如结构体A,采用字节对齐，只需要两次就可以将a,b,c加载到内存。对于结构B,则需要三次将a,b,c加载
    到内存。对于B虽然对齐前后访存此处没变，但是每次偏移固定的量，寻址速度加快了。
    
    
    
    