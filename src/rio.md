
## 读写抽象层


#### _rio结构体
        
    struct _rio {
   
    size_t (*read)(struct _rio *, void *buf, size_t len);
    size_t (*write)(struct _rio *, const void *buf, size_t len);
    off_t (*tell)(struct _rio *);
    int (*flush)(struct _rio *);
    
    void (*update_cksum)(struct _rio *, const void *buf, size_t len);

    /* 当前校验和和标记 (see RIO_FLAG_*) */
    uint64_t cksum, flags;

    /* 读取或写入的字节数 */
    size_t processed_bytes;

    /* 读写块的最大值 */
    size_t max_processing_chunk;

    /* Backend-specific vars. */
    union {
        /* 定义buffer大小. */
        struct {
            sds ptr;
            off_t pos;
        } buffer;
        /* 文件结构体. */
        struct {
            FILE *fp;//文件描述符
            off_t buffered; /*上次异步写入的字节数目. */
            off_t autosync; /* fsync after 'autosync' bytes written. */
        } file;
        /* Connection object (used to read from socket) */
        //用来连接socket的结构体
        struct {
            connection *conn;   /* Connection */
            off_t pos;    /* pos in buf that was returned */
            sds buf;      /* buffered data */
            size_t read_limit;  //允许buffer读取的最大值
            size_t read_so_far; /* 目前已经从roi中读出的数据 */
        } conn;
        /* FD target (used to write to pipe). */
        struct {
            int fd;       /* File descriptor. */
            off_t pos;
            sds buf;
        } fd;
    } io;
};

#### 相关函数
        
        size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
        参数
        ptr -- 这是指向要被写入的元素数组的指针。
        size -- 这是要被写入的每个元素的大小，以字节为单位。
        nmemb -- 这是元素的个数，每个元素的大小为 size 字节。
        stream -- 这是指向 FILE 对象的指针，该 FILE 对象指定了一个输出流。
        
        C 库函数 int fflush(FILE *stream) 刷新流 stream 的输出缓冲区。

        声明
        下面是 fflush() 函数的声明。
        
        int fflush(FILE *stream)
        参数
        stream -- 这是指向 FILE 对象的指针，该 FILE 对象指定了一个缓冲流。
        
        描述
        C 库函数 size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) 从给定流 stream 读取数据到 ptr 所指向的数组中。
        
        声明
        下面是 fread() 函数的声明。
        
        size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
        参数
        ptr -- 这是指向带有最小尺寸 size*nmemb 字节的内存块的指针。
        size -- 这是要读取的每个元素的大小，以字节为单位。
        nmemb -- 这是元素的个数，每个元素的大小为 size 字节。
        stream -- 这是指向 FILE 对象的指针，该 FILE 对象指定了一个输入流。
        
        描述
        C 库函数 long int ftell(FILE *stream) 返回给定流 stream 的当前文件位置。
        
        声明
        下面是 ftell() 函数的声明。
        
        long int ftell(FILE *stream)
        参数
        stream -- 这是指向 FILE 对象的指针，该 FILE 对象标识了流。
        返回值
        该函数返回位置标识符的当前值。如果发生错误，则返回 -1L，全局变量 errno 被设置为一个正值。
        
        
        描述
        C 库函数 int snprintf(char *str, size_t size, const char *format, ...) 设将可变参数(...)按照 format 格式化成字符串，并将字符串复制到 str 中，size 为要写入的字符的最大数目，超过 size 会被截断。
        
        声明
        下面是 snprintf() 函数的声明。
        
        int snprintf ( char * str, size_t size, const char * format, ... );
        参数
        str -- 目标字符串。
        size -- 拷贝字节数(Bytes)。
        format -- 格式化成字符串。
        ... -- 可变参数。