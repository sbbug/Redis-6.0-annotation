
## 一般的分布式锁
    
#### 基于setnx的命令
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

    
#### 基于set key value EX PX NX XX 的命令
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

    
    使用Lua脚本释放锁:
        if redis.call("get",KEYS[1]) == ARGV[1] then
            return redis.call("del",KEYS[1])
        else
            return 0
        end
    使用这种方式释放锁可以避免删除别的客户端获取成功的锁
    
#### 单点问题
    
    使用Redis服务器实现分布式锁，存在单点挂掉的问题。
    可以增加一个slave节点来实现。但是Redis中master与slave之间的同步通常是异步，
    并没有绝对意义上的同步。
    
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