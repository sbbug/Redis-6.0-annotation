

## Redis事物原理:

#### 事务相关命令

    multi 标记一个事物开始
    exec  执行队列里所有命令
    watch 监视一个或者多个key,如果在事物执行之前这个key被其它命令修改。
    那么事务将不被执行。
    unwatch 取消对key的监视
    discard 取消事物，放弃执行。刷新命令队列。
    
    Redis事物不支持回滚。

    用法：
    > MULTI  // 开启事务
    OK
    > INCR foo
    QUEUED
    > INCR bar
    QUEUED
    > EXEC // 执行事务
    1) (integer) 1
    2) (integer) 1
    
#### 事务执行机制

    所有在队列里的命令都将被串行化分别执行。同时在执行事物命令的过程中，另一个客户端发送的命令
    将不会被执行。保证了执行的隔离性。

#### 事务出错
    
    Redis事务出错分两种，一种是事务队列中语法存在错误，此时所有命令都不会执行。
    另一种场景是队列中不存在语法错误，但语义有错，此时即使出错，其它命令也不会回滚。

    1、事务中的命令存在语法错误
    这种错误Redis会在客户端提前进行校验，如果出错，直接报错。
        MULTI
        +OK
        INCR a b c
        -ERR wrong number of arguments for 'incr' command
    2、命令执行过程中出错
    即使某个命令出现错误，仍然不影响其它命令执行。

    Trying 127.0.0.1...
    Connected to localhost.
    Escape character is '^]'.
    MULTI
    +OK
    SET a abc // 执行set命令
    +QUEUED
    LPOP a  // 弹出元素 错误操作 这种不属于语法错误，属于语义错误，因此Redis前端返回queued
    +QUEUED
    EXEC
    *2
    +OK // 返回执行OK
    -ERR Operation against a key holding the wrong kind of value // 同时会有一个错误附加
    
#### 乐观锁
    
    watch对于Redis事物提供了一个CAS机制
    使用watch来监听指定的key,如果在exec之前，某个key被修改了，那么整个事物终止执行。
    和Java中CAS(先比较再交换)机制非常相似。
    watch机制主要考虑多个客户端同时操作一个key，就会产生竞争条件，因此需要确保key的安全性。

    没有引入watched命令：
        val = GET mykey // 获取key值
        val = val + 1 // 修改值
        SET mykey $val // 更新key,注意这里可能存在问题，如果此时另一个请求在val = Get mykey 之后，修改了key,
                      // 那么当前请求拿到的就是脏数据，因此修改有问题
    正确方式
        WATCH mykey // 监控key的状态变化
        val = GET mykey
        val = val + 1
        MULTI // 开启事务
        SET mykey $val
        EXEC // 提交事务
    如果mykey值发生变化，则会持续执行上述命令，直到成功。
     
#### Redis事务为什么不支持回滚操作

    作者认为Redis支持回滚操作会影响线上性能。并且Redis执行过程中如果是人为犯错，是不应该支持回滚操作的。
    如果想撤销某个命令，只能由用户通过执行其它对等命令实现。

#### 参考资料

[Redis事务](https://redis.io/topics/transactions)
    
 
    
    
    
    