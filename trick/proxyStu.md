

## Redis Proxy

#### 分片原理

        redis 中使用分片实现数据的负载均衡。具体来说Redis将key通过CRC16算法计算哈希值，然后将计算后的哈希值
    对16384取模，这样将数据压缩在[0,16383]区间内，类似hashmap桶实现原理。每个key落到对应的桶中。Redis将这
    16384个均衡的分配到不同的master节点上，实现负载均衡。每个节点负责一部分桶的信息。同时每个节点会保持一个桶和master
    对应的列表。
        如果有请求到一个master，但是key不在当前master中，那么master会在映射表中找到该桶的存在节点master，然后将请求转发过去。

#### Redis为什么要引入proxy
    
        如上所述，Redis分片基于桶实现，但是随着数据量的不断增大，key的hash值冲突越来越多，那么每个桶对应的长度
    会越来越长，严重影响数据的查询性能，因此Redis引入proxy代理层，master集群和16384个桶统称为一个cluster。
    proxy实现请求在cluster之间的调度。实现数据存储的弹性收缩。

#### 参考资料

  [redis官方temproxy](https://redis.io/topics/partitioning)  
  [redis cluster原理](https://redis.io/topics/cluster-tutorial)  
  [redis cluster特点](https://redis.io/topics/cluster-spec)  
  