## redis-check-aof模块:
              用来进行日志修复



Redis文件结构:
    REdis-4.0之前的AOF文件没有文件头，而从REdis-4.0开始AOF文件带有一个文件头，文件头格式和RDB文件头相同。
    REdis-4.0版本，如果开启aof-use-rdb-preamble（值为yes，5.0默认为yes，4.0默认为no），则AOF文件内容由三部分组成：
    1) 文件头（和RDB文件头格式相同）
    2) RDB格式的二进制数据段
    3) AOF格式的文本数据段

## 宏定义
      #define redis_fstat fstat64
      #define redis_stat stat64

fileno:
    函数名称：fileno（在VC++6.0下为_fileno）
    函数原型：int _fileno( FILE *stream );
    函数功能：fileno()用来取得参数stream指定的文件流所使用的文件描述符
    返回值：某个数据流的文件描述符
    头文件：stdio.h
    相关函数：open，fopen，fclose
ftello：
rewind：
    C 库函数 void rewind(FILE *stream) 设置文件位置为给定流 stream 的文件的开头。
    重新将文件读取位置定位到开头