# Redis6.0 源码阅读
## 学习编程思想，先从优秀的源码阅读开始
## Redis-6.0-annotation
This is the Chinese annotation of redis-6.0
going

## Redis is named REmote DIctionary Server

第一阶段
阅读Redis的数据结构部分，基本位于如下文件中： 
+ 内存分配 zmalloc.c和zmalloc.h 
+ 动态字符串 sds.h和sds.c [sds](./src/sds.md)
+ 双端链表 adlist.c和adlist.h [adlist](./src/adlist.md)
+ 字典 dict.h和dict.c [dict](./src/dict.md)
+ 跳跃表 server.h文件里面关于zskiplist结构和zskiplistNode结构，以及t_zset.c中所有zsl开头的函数，比如 zslCreate、zslInsert、zslDeleteNode等等。 
+ 日志类型 hyperloglog.c 中的 hllhdr 结构， 以及所有以 hll 开头的函数

第二阶段
熟悉Redis的内存编码结构 
+ 整数集合数据结构 intset.h和intset.c [intset](./src/inset.md)
+ 压缩列表数据结构 ziplist.h和ziplist.c [ziplist](./src/ziplist.md)

第三阶段
熟悉Redis数据类型的实现 
+ 对象系统 object.c [object](./src/object.md)
+ 字符串键 t_string.c 
+ 列表建 t_list.c [t_list](./src/t_list.md)
+ 散列键 t_hash.c 
+ 集合键 t_set.c 
+ 有序集合键 t_zset.c中除 zsl 开头的函数之外的所有函数 
+ HyperLogLog键 hyperloglog.c中所有以pf开头的函数

第四阶段
熟悉Redis数据库的实现 
+ 数据库实现 redis.h文件中的redisDb结构，以及db.c文件 
+ 通知功能 notify.c 
+ RDB持久化 rdb.c [rdb](./src/rdb.md)
+ AOF持久化 aof.c [aof](./src/aof.md)

以及一些独立功能模块的实现 
+ 发布和订阅 redis.h文件的pubsubPattern结构，以及pubsub.c文件 
+ 事务 redis.h文件的multiState结构以及multiCmd结构，multi.c文件

第五阶段
熟悉客户端和服务器端的代码实现 
+ 事件处理模块 ae.c/ae_epoll.c/ae_evport.c/ae_kqueue.c/ae_select.c 
+ 网路链接库 anet.c和networking.c 
+ 服务器端 redis.c 
+ 客户端 redis-cli.c [redis-cli](./src/redis-cli.md)

这个时候可以阅读下面的独立功能模块的代码实现 
+ lua脚本 scripting.c 
+ 慢查询 slowlog.c 
+ 监视 monitor.c

第六阶段
这一阶段主要是熟悉Redis多机部分的代码实现

复制功能 replication.c
Redis Sentinel sentinel.c
集群 cluster.c
其他代码文件介绍
关于测试方面的文件有： 
+ memtest.c 内存检测 
+ redis_benchmark.c 用于redis性能测试的实现。 
+ redis_check_aof.c 用于更新日志检查的实现。 [redis_check_aof](./src/redis-check-aof.md)
+ redis_check_dump.c 用于本地数据库检查的实现。 
+ testhelp.c 一个C风格的小型测试框架。

一些工具类的文件如下： 
+ bitops.c GETBIT、SETBIT 等二进制位操作命令的实现 
+ debug.c 用于调试时使用 
+ endianconv.c 高低位转换，不同系统，高低位顺序不同 
+ help.h 辅助于命令的提示信息 
+ lzf_c.c 压缩算法系列 
+ lzf_d.c 压缩算法系列 
+ rand.c 用于产生随机数 
+ release.c 用于发布时使用 
+ sha1.c sha加密算法的实现 
+ util.c 通用工具方法 
+ crc64.c 循环冗余校验 
+ sort.c SORT命令的实现

一些封装类的代码实现： 
+ bio.c background I/O的意思，开启后台线程用的 [bio](./src/bio.md)
+ latency.c 延迟类 [latency](./src/latency.md)
+ migrate.c 命令迁移类，包括命令的还原迁移等 
+ pqsort.c 排序算法类 
+ rio.c redis定义的一个I/O类 [rio](./src/rio.md)
+ syncio.c 用于同步Socket和文件I/O操作 


## 参考
1、[redis内存淘汰方式](http://oldblog.antirez.com/post/redis-as-LRU-cache.html)

2、[从0到1构建高可用Redis](https://zhaoxiaobin.net/Redis/%E5%A6%82%E4%BD%95%E4%BB%8E0%E5%88%B01%E6%9E%84%E5%BB%BA%E4%B8%80%E4%B8%AA%E7%A8%B3%E5%AE%9A%E3%80%81%E9%AB%98%E6%80%A7%E8%83%BD%E7%9A%84Redis%E9%9B%86%E7%BE%A4%EF%BC%9F%EF%BC%88%E8%BD%AC%E8%BD%BD%EF%BC%89/)  

3、[Jedis源码阅读笔记](https://gitee.com/sbbug/jedis)  

## 博大精深，每次捋一遍都有不一样的体会哦！！    
    
    
        
