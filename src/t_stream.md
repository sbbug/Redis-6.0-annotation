## Redis-stream 消息队列

    Redis的流就是专门为时序数据设计的。
    主线是一个消息链表，将所有消息按照顺序串起来。
    因此Redis的流在这方面支持度是非常不错的，我们可以将平面内的数据按照时间序列加入到Redis流里面。
    stream是一个可持久化的消息队列， 对标kafka，解决了pubsub订阅发布功能不能持久化的问题。

#### 使用方式

###### 发布消息     
     添加一条消息流，类似一个主题:
     xadd stream_name id key-value [key-value,...]
     stream_name:消息流名称
     id:一般设置为*,系统自动生成。 <millisecondsTime>-<sequenceNumber>
     key value:流中的某一条数据
     
     查询消息队列长度：
     xlen stream_name 
     删除队列消息：
     XDEL key ID [ID …]
     
###### 订阅消息     
     按照范围查询消息
     xrange stream_name -(start) +(end)  //-表示最小值 +表示最大值
     
     xrange stream_name - + count 2
     
     监听模式获取消息
     非阻塞
     xread count 4 streams stream_name 0 //从stream中拿ID比0大的4个entry，按照升序排列
     阻塞
     xread block 0 streams strean_name $ //监听stream_name,从stream_name拿比$还要大的entry,
     以此来实现阻塞，如果比当前最大ID小，则满足条件不阻塞。类似zookeeper里的watch。
     
     block 0:无限阻塞，不超时，如果大于0则为超时时间。
###### 消费者组

     一个stream的数据每个消费者组都发一份，同组消费者竞争同一个数据。
     消费者组需要以下5点保证:
     1、组内消费者消费的消息不重复
     2、组内消费者名称必须唯一
     3、消费者成功消费消息需要返回ACK,然后这条消息才会从消费者组中移除。
     4、group是在服务端的结构，应该是分发。不是消费
     5、consumer通过唯一标识认定，即使换了客户端，只要ID相同，group仍然认为是同一个consumer。
     
     创建消费者组:
     xgroup create stream_name group_name $
     使用xgroup命令创建一个group_name消费者组，该消费者组和stream_name绑定。
     以后group_name组中的消费者就会消费stream_name中的数据。
     
     从消费者组中读数据:
     xreadgroup group group_name  consumer_a count 1 streams stream_name >
     //使用xreadgroup命令让消费者consumer_a从group_name消费者组的stream_name中拿到最新的，
     并且没有发送给消费者处理的entry.
     
     发送ACK:
     将指定的ID对应的entry从consumer的已经处理的消息列表删除
     xack stream_name group_name 1432543636-0
     
      

#### 实现原理

###### stream id结构
    typedef struct streamID {
        uint64_t ms;        /* unix时间戳，毫秒为单位. */
        uint64_t seq;       /* 序列号. */
    } streamID;
###### stream 结构

    typedef struct stream {
    rax *rax;               //使用基数树存储内容
    uint64_t length;        //消息数量
    streamID last_id;       //最新ID
    rax *cgroups;           //消费者组
    } stream;

####### 消费者组结构
    typedef struct streamCG {
        streamID last_id;       /* Last delivered (not acknowledged) ID for this
                                   group. Consumers that will just ask for more
                                   messages will served with IDs > than this. */
        rax *pel;               /* Pending entries list. This is a radix tree that
                                   has every message delivered to consumers (without
                                   the NOACK option) that was yet not acknowledged
                                   as processed. The key of the radix tree is the
                                   ID as a 64 bit big endian number, while the
                                   associated value is a streamNACK structure.*/
                                   
        rax *consumers;         //消费者集合
    } streamCG;
###### 消费者结构

    typedef struct streamConsumer {
    mstime_t seen_time;         //消费者最后活跃时间
    sds name;                   //消费者名字
    rax *pel;                   /* Consumer specific pending entries list: all
                                   the pending messages delivered to this
                                   consumer not yet acknowledged. Keys are
                                   big endian message IDs, while values are
                                   the same streamNACK structure referenced
                                   in the "pel" of the conumser group structure
                                   itself, so the value is shared. */
    } streamConsumer;

## 参考
    https://cloud.tencent.com/developer/article/1529507