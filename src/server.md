server.c main方法启动流程:

    1.REDIS_TEST是否模块测试，如果是返回测试结果
    2.initServerConfig();//初始化服务器的一些属性信息
      ACLInit(); //初始化ACL验证信息
      moduleInitModulesSystem();//初始化模块，加载动态链接库
      tlsInit();//tls信息初始化
    3. if (server.sentinel_mode) {
        initSentinelConfig();
        initSentinel();
        }//哨兵模式初始化
    4.redis_check_rdb_main(argc,argv,NULL);//核查是否需要初始化rdb
      redis_check_aof_main(argc,argv);//核查是否需要初始化aof
    5.initServer();//初始化服务器
    6.if (!server.sentinel_mode) {//是否开启哨兵模式
        /* Things not needed when running in Sentinel mode. */
        serverLog(LL_WARNING,"Server initialized");
        #ifdef __linux__
            linuxMemoryWarnings();
        #endif
            moduleLoadFromQueue();
            ACLLoadUsersAtStartup();
            InitServerLast();
            loadDataFromDisk();
            ......
        } else {
            InitServerLast();//服务器后续初始化步骤
            sentinelIsRunning();
        }
    7. aeSetBeforeSleepProc(server.el,beforeSleep);//设置进入epoll前方法
       aeSetAfterSleepProc(server.el,afterSleep);//设置进去epoll后方法
    8. aeMain(server.el);//进去事件循环处理器
    9. aeDeleteEventLoop(server.el);//释放事件处理器,服务运行结束
    
Redis哨兵模式(Redis Sentinel):

    Redis哨兵是用来监控、管理其它Redis实例的工具。具体功能如下:
    1.监控其它节点是否正常运行，包括master、slave
    2.通知，当某个节点出现故障，哨兵会通知其它节点
    3.自动故障转移操作，当集群master出故障后，哨兵会将与出故障的主节点相连的其它
    从节点中一个节点转换为主节点，并将其它从节点指向它
    4.为了真正达到高可用，哨兵一般会有多个节点
    
Redis中的TLS：

    最新版本的TLS（Transport Layer Security，传输层安全协议）是IETF（Internet Engineering Task Force，Internet工程任务组）
    制定的一种新的协议，它建立在SSL 3.0协议规范之上，是SSL 3.0的后续版本。
    在TLS与SSL3.0之间存在着显著的差别，主要是它们所支持的加密算法不同，所以TLS与SSL3.0不能互操作。
Redis的key的TTL：

    通过TTL命令返回key的过期时间。
    当key没有设置过期时间返回-1
    当key设置过期时间，且已经过期，返回-2
    当key设置过期时间，且未过期，返回剩余时间



  

小计:
    extern关键字:
        extern关键字可以用来声明变量和函数作为外部变量或者函数供其它文件使用。
    strcasecmp:
        定义函数 int strcasecmp (const char *s1, const char *s2);
        函数说明 strcasecmp()用来比较参数s1和s2字符串，比较时会自动忽略大小写的差异。
        返回值 若参数s1和s2字符串相等则返回0。s1大于s2则返回大于0 的值，s1 小于s2 则返回小于0的值
        
    signal(SIGHUP, SIG_IGN)
    
    signal(SIGPIPE, SIG_IGN):
        server端进程启动中常见的signal(SIGPIPE， SIG_IGN)信号屏蔽
        原因是在socket编程中，若client端已经关闭，继续对其进行write操作，会产生broken_pipe信号，导致进程退出。