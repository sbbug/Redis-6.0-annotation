
## 关于主从复制模式的一些常见问题:

#### 1、主从复制模式中从节点如何处理过期的key?
    
    master节点检测到有key过期时，会主动通知slave节点。
    然后slave节点对过期的key进行删除。
    
#### 2、Redis主从复制采用异步还是同步?

       异步复制
    
#### 3、Redis复制是如何工作的?
   
    每个master有一个replication id和offset,用来标识master数据库
    的状态。
    具体步骤如下:
    a、slave使用PSYNC连接master，携带自身旧的rep-id和offset发送给
    master节点，master节点进行判断，是否rep-id、offset与它自身的一致，如果一致
    简单增加即可，如果不一致，需要将slave重新更新。
    b、master节点执行bgsave,将当前内存数据生成RDB文件，同时把客户端写命令缓存到
    buffer
    c、当rdb文件完成后，master将其发给slave，slave将rdb作为初始化数据加载。同时
    master节点将缓存写命令发送给slave去执行。以此来保持同步。
    
#### 4、slave数据为什么是read-only mode?
    
    在redis.conf文件中，我们通过参数replica-read-only设置为只读模式
    