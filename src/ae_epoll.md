
## Linux epoll机制


#### 创建epoll实例
    int epoll_create(int flags);
    创建一个epoll的句柄，size用来告诉内核这个监听的数目最大值。
    
#### epoll管理事件
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