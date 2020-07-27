
## redis 哨兵模式中的常见问题

#### 1、哨兵作用?

    监视、通知、容错、
    
#### 2、SDOWN与ODOWN区别？

    SDOWN大意是某个哨兵主观认为某个节点下线。
    ODOWN是哨兵通过征询其它节点对故障节点的反馈后，客观认为故障节点下线。
    SDOWN是节点没有在规定时间内回复pong。
    从SDOWN到ODOWN没有使用强一致性算法，直接使用gossip。
    一般来说认为节点主观下线的哨兵会征求其它哨兵意见，如果大多数哨兵认为该节点已经下线，
    那么就会将该节点标记为客观下线。
    
    
    
    
#### 3、哨兵与哨兵、哨兵与slave节点之前的自动发送功能如何实现？
    
     基于Redis的发布-订阅模式，使用__sentinel__:hello频道可以实现
     节点之间的数据交换。
     每个哨兵会向被监视的master-slave集群的__sentinel__:hello频道发布和订阅消息，
     宣示哨兵的存在(ip,端口号)等，同时哨兵也可以获取到每个master的基本信息，包括
     属性slaveof。
     
#### 4、哨兵是如何实现容错功能的?
        
      当哨兵发现master掉线后，会从slave中选择一个master出来。
      slave被选为master需要满足如下条件:
      a、上一次同master的断开时间
      b、副本的优先权
      c、副本处理的offset
      d、run id
      因此会将满足条件的slave进行排序，首先按照redis.conf中的
      replica-prority属性进行排序；如果属性相同，根据offset进行排序，
      offset大的排在前面，offset越大说明同步数据越多；如果属性相同且offset
      相同，则将run ID较小的排在前面。

    
  