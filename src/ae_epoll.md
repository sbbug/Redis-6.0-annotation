
## Linux epoll机制


#### 创建epoll实例
    //创建一个事件空间
    int epoll_create(int flags);
    创建一个epoll的句柄，size用来告诉内核这个监听的数目最大值。
    
#### epoll管理事件
    //该方法主要用来实现事件的注册功能
    int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
    输入:
        epfd是epoll的句柄，即epoll_create的返回值；
        op表示动作：用三个宏表示：
        EPOLL_CTL_ADD：注冊新的fd到epfd中；
        EPOLL_CTL_MOD：改动已经注冊的fd的监听事件；
        EPOLL_CTL_DEL：从epfd中删除一个fd；
        fd是须要监听的套接字描写叙述符；
        event是设定监听事件的结构体，数据结构例如以下：
        
#### 等待epoll事件
    int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
    等待事件的产生，返回须要处理的事件的数量，并将需处理事件的套接字集合于參数events内，能够遍历events来处理事件
    输入:
        epfd : epoll实例的fd
        events : 储存事件的数组首地址
        maxevents : 最大事件的数量
        timeout : 等待的最长时间
        
    Linux中epoll首先使用epoll_create方法开辟一个事件空间，创建组织文件描述符的红黑树，以及就绪事件的链表，凡是有就绪的事件
    ，都会拷贝到链表中。
    然后当有新的事件到达后，使用epoll_ctl方法将事件注册到内核空间中，在内核空间中，所有的文件描述符是以红黑树的结构组织存储，就绪
    事件拷贝到链表中。所有的文件描述符只从用户态拷贝到内核态一次，不会出现重复拷贝，这也是epoll性能高的原因。同时epoll机制在
    内核态创建一个链表，用来存储就绪的文件描述符事件，每次只需要将已经就绪的事件通过epoll_wait方法拷贝到用户态
    即可，不需要拷贝所有的文件描述。

#### 参考
    1、https://www.cnblogs.com/codestack/p/13393658.html
    2、https://www.cnblogs.com/ym65536/p/7750312.html