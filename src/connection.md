connection是对anet模块的进一步封装。

用到的关键技术:
    
    基于函数类型的变量
    //声明一个ConnectionCallbackFunc函数指针类型
    typedef void (*ConnectionCallbackFunc)(struct connection *conn);
    //在结构体中定义该变量
    struct connection {
        ConnectionType *type;//连接类型
        ConnectionState state;//连接状态
        ......
        ConnectionCallbackFunc conn_handler;
        ConnectionCallbackFunc write_handler;
        ConnectionCallbackFunc read_handler;
        int fd;//文件描述符
    };
    
结构体
     
     声明一个结构体   
     ConnectionType CT_Socket;
     初始化一个结构体
     
    //对应ConnectionType CT_Socket ，对其进行赋值初始化
    ConnectionType CT_Socket = {
        .ae_handler = connSocketEventHandler,
        .close = connSocketClose,
        .write = connSocketWrite,
        .read = connSocketRead,
        .accept = connSocketAccept,
        .connect = connSocketConnect,
        .set_write_handler = connSocketSetWriteHandler,
        .set_read_handler = connSocketSetReadHandler,
        .get_last_error = connSocketGetLastError,
        .blocking_connect = connSocketBlockingConnect,
        .sync_write = connSocketSyncWrite,
        .sync_read = connSocketSyncRead,
        .sync_readline = connSocketSyncReadLine
    };
     