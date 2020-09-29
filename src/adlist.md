#### redis链表

      Redis中链表主要是基于双向链表，以及链表相关常用的API.
      
#### 链表内部结构

    typedef struct listNode {//双向链表的节点
        struct listNode *prev;
        struct listNode *next;
        void *value;
    } listNode;
    
    typedef struct listIter {//链表迭代器
        listNode *next;
        int direction;
    } listIter;//链表迭代器实现对链表的封闭式访问，屏蔽内部实现细节
    
    typedef struct list {//链表对象
        listNode *head;
        listNode *tail;
        void *(*dup)(void *ptr);
        void (*free)(void *ptr);//函数指针
        int (*match)(void *ptr, void *key);
        unsigned long len;//长度
    } list;
    
    使用迭代器访问的好处可以屏蔽内部实现细节，对用户是透明的。