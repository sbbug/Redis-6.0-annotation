
## 一般的分布式锁

### redis的分布式锁

#### 基于setnx的命令设置分布式锁
    setnx key value 
    expire key seconds
    这两个命令非原子性，因此不安全的。如果设置setnx后，后端崩溃，那么将出现死锁问题。
    
    需要使用Lua脚本实现:
        public boolean tryLock_with_lua(String key, String UniqueId, int seconds) {
            String lua_scripts = "if redis.call('setnx',KEYS[1],ARGV[1]) == 1 then" +
                    "redis.call('expire',KEYS[1],ARGV[2]) return 1 else return 0 end";
            List<String> keys = new ArrayList<>();
            List<String> values = new ArrayList<>();
            keys.add(key);
            values.add(UniqueId);
            values.add(String.valueOf(seconds));
            Object result = jedis.eval(lua_scripts, keys, values);
            //判断是否成功
            return result.equals(1L);
        }
    Lua脚本执行属于原子性操作，可有效避免死锁问题。

#### 基于set key value EX/PX NX/XX 的命令设置分布式锁
    value可以标记操作者的唯一性
    EX seconds设置键的过期时间为秒
    PX millisecounds 设置键的过期时间为毫秒
    NX 键不存在时才进行设置操作，保证键的唯一性
    XX 只有键存在时才进行设置操作
    设置具有过期时间的唯一key
    set k 100 EX 10 NX //设置k为100，且存活时间为10s 并且唯一。
    
    加锁代码:
        public boolean tryLock_with_set(String key, String UniqueId, int seconds) {
            return "OK".equals(jedis.set(key, UniqueId, "NX", "EX", seconds));
        }



#### 分布式锁的释放
    对于分布式锁的释放问题，如果简单的del删除操作，有可能会误删其它client设置的锁，考虑一种场景：A请求设置锁成功，并执行释放锁的操作，结果由于网络阻塞等原因，锁已经过期了，并且被其它请求拿到锁，网络阻塞恢复后，请求到达，又删除了别人加的锁，乌龙了！！  
    因此释放锁时做一层判断，只有持有者才可以释放锁，可以把持有者ID存储在value中。为了保证操作的原子性，使用LUA脚本实现。
    使用Lua脚本释放锁:
        if redis.call("get",KEYS[1]) == ARGV[1] then
            return redis.call("del",KEYS[1])
        else
            return 0
        end
    使用这种方式释放锁可以避免删除别的客户端获取成功的锁
    
    关于LUA脚本如何保证操作原子性的，参考这里[]()

#### 单点问题

    使用Redis服务器实现分布式锁，存在单点挂掉的问题。
    可以增加一个slave节点来实现。但是Redis中master与slave之间的同步通常是异步，
    并没有绝对意义上的同步。任何一个分布式系统，主从节点之间数据同步都会有延迟，这个状态
    叫soft state

#### RedLock算法

    假设集群中存在五个master节点。
    1、获取当前Unix时间，以毫秒为单位。
    2、依次尝试从N个实例，使用相同的key和随机值获取锁。
        在步骤2，当向Redis设置锁时,客户端应该设置一个超时时间，
        这个超时时间应该小于锁的失效时间。例如锁自动失效时间为10秒，则超时时间在5-50毫秒之间。
        这样可以避免服务器端Redis已经挂掉的情况下，客户端还在死死地等待响应结果。
        如果服务器端没有在规定时间内响应，客户端应该尽快尝试另外一个Redis实例。
    3、客户端使用当前时间减去开始获取锁时间（步骤1记录的时间）就得到获取锁使用的时间。
        当且仅当从大多数（这里是3个节点）的Redis节点都取到锁，并且获取锁使用的时间小于锁失效时间时，锁才算获取成功。
    4、如果取到了锁，key的真正有效时间等于有效时间减去获取锁所使用的时间（步骤3计算的结果）。
    5、如果因为某些原因，获取锁失败（没有在至少N/2+1个Redis实例取到锁或者取锁时间已经超过了有效时间），
        客户端应该在所有的Redis实例上进行解锁（即便某些Redis实例根本就没有加锁成功）。

###### RedLock算法可能存在的一些问题：

      1、假设三个节点node1,node2,node3,同时过来两个请求r1,r2,如果r1此时获取node1、node2节点的锁，r2获取node3节点的锁。
         那么此时r1获取锁成功，r2获取锁失败。正常工作。如果此时来了三个请求r1,r2,r3,他们分别获取了node1,node2,node3节点的锁，
         然后出现死锁问题，造成互相等待。这个问题也好解决，每个锁有一个过期时间。锁过期之后，再重新获取锁。

#### 分布式锁下面如何增大请求的并发性

      1、可以从加锁粒度出发，细化加锁粒度。比如一个商品数量是100件，可以将100分成两个区间加锁1~50 51~100,这样可以同时支持
      两个线程访问。当然还可以更细粒度划分。
      2、在zookeeper实现分布式锁
        首先请求到达，到对应的锁节点下注册一个临时顺序节点，然后判断注册的节点是不是最小节点，如果是则获取锁成功。
        如果不是则获取锁失败，同时对前面的最后一个节点注册监听，直到被唤醒。  
      3、采取读写方式加锁，业务场景多为读多写少场景，因此分读锁与写锁两种方式。
#### 分布式锁保质期过短问题

      在分布式系统中，如果A用户获取分布式锁，锁的过期时间是10s,但是由于A用户执行时间比较长，
      导致还未结束，锁已经过期被自动释放掉。
      考虑解决方案：
      1、当用户A获取分布式锁后，设置锁的value为用户ID，同时为该获取锁的用户分配一个守护线程。
      守护线程可以每隔8s，去检测一下该用户所持有的锁是否存在，如果存在，说明该用户执行时间比较长，可能
      还需要时间，因此对锁续期，重置时间为10s。
      Redisson插件中有提供API。
      2、...          
  
    
## 其它工具实现分布式锁

### MySQL数据库实现分布式锁

   可以使用MySQL的字段的唯一性约束功能实现分布式锁。也就是说一张表中某个字段不可以出现重复值。

   创建数据表  

           CREATE TABLE `distributed_lock` (
           `id` bigint(20) NOT NULL AUTO_INCREMENT,
           `unique_mutex` varchar(255) NOT NULL COMMENT '业务防重id',
           `holder_id` varchar(255) NOT NULL COMMENT '锁持有者id',
           `create_time` datetime DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
           PRIMARY KEY (`id`),
           UNIQUE KEY `mutex_index` (`unique_mutex`)
           ) ENGINE=InnoDB DEFAULT CHARSET=utf8;
   执行获取锁的操作
    
        获取锁:
        insert into distributed_lock(unique_mutex, holder_id) values ('unique_mutex', 'holder_id');
        //如果插入成功，则获取该锁
   执行释放锁操作

        释放锁:
        delete from methodLock where unique_mutex='unique_mutex' and holder_id='holder_id';
        //释放该锁
   
   考虑几个问题

   1、分布式锁创建后，如何保证锁的可重入
     使用hold_id判断
   2、锁创建后，如果释放锁操作失败，该怎么办
     客户端可以使用重试方法
     其它客户端端在加锁之前，判断记录是否存在，如果存在，计算锁的创建时间和当前时间时差，如果大于某个阈值，直接过期。

### zookeeper实现分布式锁

    zookeepr中数据存储是一个树结构，节点为ZNode节点，具有持久节点、持久顺序节点、临时节点、临时顺序节点。

    使用临时顺序节点实现分布式锁的创建。

#### 完全分布式锁

    同一时间，无论读请求还是写请求只能有一个请求拿到锁。
    1、调用create()方法在parentLock路径下创建一个临时顺序节点，并被分配序号
    2、调用getChildren()方法获取父节点下的所有子节点，并对节点按照序号排序
    3、如果创建的临时节点序号等于子节点集合的最小序号，那么获取锁成功
    4、如果不是最小节点，客户端会调用exists()方法为前一个小节点注册一个watch监听器，如果返回为null，说明前一个节点已经
       不存在，则重新进入第二步去获取锁。否则进入等待状态，直到上一个节点删除，再重新进入第二步。

    代码实现：

	public void lock(){
		
		List<String> allCompetitorList = null;
		try {
		    createComPrtitorNode(); // 请求到达，创建临时顺序节点
			allCompetitorList = pandaZk.getChildren(lockPath, false); // 获取所有子节点
		} catch (KeeperException e) {
			throw new PandaLockException("zookeeper connect error");
		} catch (InterruptedException e) {
			throw new PandaLockException("the lock has  been Interrupted");
		}
		    Collections.sort(allCompetitorList); // 节点排序 
		int index = allCompetitorList.indexOf(thisCompetitorPath
				.substring((lockPath + SEPARATOR).length()));
		if (index == -1) {
			throw new PandaLockException("competitorPath not exit after create");
		} else if (index == 0) {// 如果发现自己就是最小节点,那么说明本人获得了锁
			return;
		} else {// 说明自己不是最小节点
            // 获取我前面的最小节点
			waitCompetitorPath = lockPath+SEPARATOR + allCompetitorList.get(index - 1);
             // 在waitPath上注册监听器, 当waitPath被删除时, zookeeper会回调监听器的process方法
            Stat waitNodeStat;
			try {
                // 对前面的最小节点注册一个监听器
				waitNodeStat = pandaZk.exists(waitCompetitorPath, new Watcher() {
					@Override
					public void process(WatchedEvent event) {
						if (event.getType().equals(EventType.NodeDeleted)&&event.getPath().equals(waitCompetitorPath)) {
							getTheLocklatch.countDown(); // 放出请求
						}
					}
				});
				 if (waitNodeStat==null) {//如果运行到此处发现前面一个节点已经不存在了。说明前面的进程已经释放了锁
		            	return;
					}else {
						getTheLocklatch.await(); // 阻塞等待 countdown()被执行
						return;
					}
			} catch (KeeperException e) {
				throw new PandaLockException("zookeeper connect error");
			} catch (InterruptedException e) {
				throw new PandaLockException("the lock has  been Interrupted");
			}
		}
	}
    
    考虑几个问题
    如果两个客户端同时获取getchild()方法，会不会出现不一致的问题，比如A请求拿到五个节点，B请求也拿到五个节点，然后排序计算，发现他们
    的序号，嗯，应该没问题，因为生成的节点序号一定不唯一。
   
    那么zookeeper如何保证生成的临时节点序号是有序唯一的呢？

#### 实现读分布式锁

    实现读分布式锁和完全分布式锁一样，只有一些轻微改动。
    1、客户端调用create方法创建一个临时顺序节点，并设置字段"read_"标识
    2、然后调用getChildren()方法，获取下面所有节点，并排序
    3、判断比创建节点小的序号节点集合是否有write字段，如果无，则获取读锁成功。
    4、如果存在write节点，则对前面靠当前节点最的write节点设置watch监听器，即调用exists()
    方法，如果方法返回为null,则直接进入第二步再次判断。否则，进入等待，监听器执行回调方法，通知阻塞的请求，
    然后再进入第二步。

#### 实现写分布式锁

    写分布式锁实现和完全分布式锁实现基本一样。

 
## 参考

[zookeeper官方介绍分布式锁]()