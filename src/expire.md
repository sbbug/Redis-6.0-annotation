
## redis 对于过期的key的删除策略

   在Redis中，我们可以设置key的过期时间，那么Redis对于key的过期时间
   主要有这么几种删除策略，分别是定时删除、惰性删除、定期删除
   
#### 定时器模式删除

    在设置键的过期时间的同时，创建一个定时器，让定时器在键的过期时间来临时，立即执行对键的删除操作。
    定时器实现方式
    


#### 惰性删除

    每次在获取key的时候，判断一下当前key是否过期，如果过期则返回null。   
    存在的问题，可能有些已经过期的key永远没有被访问过，导致迟迟不能删除。
    
    代码位置，位于db.c文件下
    //寻找某个key
    robj *lookupKeyRead(redisDb *db, robj *key) {
        return lookupKeyReadWithFlags(db,key,LOOKUP_NONE);
    }
    robj *lookupKeyReadWithFlags(redisDb *db, robj *key, int flags) {
        robj *val;
        //当前key是否过期
        if (expireIfNeeded(db,key) == 1) {
            //需要分别考虑master节点与slave节点
            /* Key expired. If we are in the context of a master, expireIfNeeded()
             * returns 0 only when the key does not exist at all, so it's safe
             * to return NULL ASAP. */
            if (server.masterhost == NULL) {
                server.stat_keyspace_misses++;
                notifyKeyspaceEvent(NOTIFY_KEY_MISS, "keymiss", key, db->id);
                return NULL;
            }
    
            /* However if we are in the context of a slave, expireIfNeeded() will
             * not really try to expire the key, it only returns information
             * about the "logical" status of the key: key expiring is up to the
             * master in order to have a consistent view of master's data set.
             *
             * However, if the command caller is not the master, and as additional
             * safety measure, the command invoked is a read-only command, we can
             * safely return NULL here, and provide a more consistent behavior
             * to clients accessign expired values in a read-only fashion, that
             * will say the key as non existing.
             *
             * Notably this covers GETs when slaves are used to scale reads. */
            if (server.current_client &&
                server.current_client != server.master &&
                server.current_client->cmd &&
                server.current_client->cmd->flags & CMD_READONLY)
            {
                server.stat_keyspace_misses++;
                notifyKeyspaceEvent(NOTIFY_KEY_MISS, "keymiss", key, db->id);
                return NULL;
            }
        }
        val = lookupKey(db,key,flags);
        if (val == NULL) {
            server.stat_keyspace_misses++;
            notifyKeyspaceEvent(NOTIFY_KEY_MISS, "keymiss", key, db->id);
        }
        else
            server.stat_keyspace_hits++;
        return val;
    }
    //判断某个key是否过期
    int expireIfNeeded(redisDb *db, robj *key) {
        if (!keyIsExpired(db,key)) return 0;
    
        /* 对于主从模式下key的过期处理，是master节点判断某个key是否过期，
         *如果过期则向slave节点发送del key命令进行删除。
         */
        if (server.masterhost != NULL) return 1;
    
        /*删除key */
        server.stat_expiredkeys++;
        propagateExpire(db,key,server.lazyfree_lazy_expire);
        notifyKeyspaceEvent(NOTIFY_EXPIRED,
            "expired",key,db->id);
        //根据配置，采取两种删除策略    
        int retval = server.lazyfree_lazy_expire ? dbAsyncDelete(db,key) ://同步删除
                                                   dbSyncDelete(db,key);//异步删除
        if (retval) signalModifiedKey(NULL,db,key);
        return retval;
    }
    //判断key是否过期
    int keyIsExpired(redisDb *db, robj *key) {
        mstime_t when = getExpire(db,key);//获取key的在哪个时间过期
        mstime_t now;
    
        if (when < 0) return 0; //不存在保质期
    
        //如果数据正在载入，不进行判断
        if (server.loading) return 0;
    
        /* If we are in the context of a Lua script, we pretend that time is
         * blocked to when the Lua script started. This way a key can expire
         * only the first time it is accessed and not in the middle of the
         * script execution, making propagation to slaves / AOF consistent.
         * See issue #1525 on Github for more information. */
        if (server.lua_caller) {
            now = server.lua_time_start;
        }
        /* If we are in the middle of a command execution, we still want to use
         * a reference time that does not change: in that case we just use the
         * cached time, that we update before each call in the call() function.
         * This way we avoid that commands such as RPOPLPUSH or similar, that
         * may re-open the same key multiple times, can invalidate an already
         * open object in a next call, if the next call will see the key expired,
         * while the first did not. */
        else if (server.fixed_time_expire > 0) {
            now = server.mstime;
        }
        /* For the other cases, we want to use the most fresh time we have. */
        else {
            now = mstime();
        }
        return now > when;//当前时间大于when时，返回真
    }
    /* 将当前key的过期信息传播到slave节点和AOF文件
     * 当master节点上某个key过期后，会把del key命令传播到slave节点，同时传给AOF文件。
     *
     * This way the key expiry is centralized in one place, and since both
     * AOF and the master->slave link guarantee operation ordering, everything
     * will be consistent even if we allow write operations against expiring
     * keys. */
     //如果集群中有key过期，需要传播到其它节点
    void propagateExpire(redisDb *db, robj *key, int lazy) {
        robj *argv[2];
    
        argv[0] = lazy ? shared.unlink : shared.del;
        argv[1] = key;
        incrRefCount(argv[0]);
        incrRefCount(argv[1]);
    
        if (server.aof_state != AOF_OFF)//如果开启了AOF
            //将过期某个key的命令写入AOF文件
            feedAppendOnlyFile(server.delCommand,db->id,argv,2);
        //在分布式存储中国，将当前删除key的命令传播给其它节点
        replicationFeedSlaves(server.slaves,db->id,argv,2);
        
        //回收内存,redis通过引用计数实现内存的自动回收与释放
        decrRefCount(argv[0]);
        decrRefCount(argv[1]);
    }
    //获取指定key的过期时间点
    long long getExpire(redisDb *db, robj *key) {
        dictEntry *de;//获取当前key对应的entry
   
        if (dictSize(db->expires) == 0 ||
           (de = dictFind(db->expires,key->ptr)) == NULL) return -1;
   
        serverAssertWithInfo(NULL,key,dictFind(db->dict,key->ptr) != NULL);
        return dictGetSignedIntegerVal(de);
    }
    //
    #define dictGetSignedIntegerVal(he) ((he)->v.s64)
    //
    typedef struct dictEntry {//定义字典所用到的k-v结构体
        void *key;
        union {//value可以选择四种类型存储
            void *val;
            uint64_t u64;
            int64_t s64;
            double d;
        } v;
        struct dictEntry *next;//指向下一个节点
    } dictEntry;

#### 定期删除

    Redis定时检查数据库，然后随机删除一些过期的健。
    Redis默认是每秒进行10次过期扫描，然后对过期的key进行删除。
    Redis 每次扫描并不是遍历过期字典中的所有键，而是采用随机抽取判断并删除过期键的形式执行的。
    
    代码：
    void activeExpireCycle(int type) {
        //主要是for循环
        for (j = 0; j < dbs_per_call && timelimit_exit == 0; j++) {
            /* Expired and checked in a single loop. */
            unsigned long expired, sampled;
    
            redisDb *db = server.db+(current_db % server.dbnum);//获取当前DB
    
            /* Increment the DB now so we are sure if we run out of time
             * in the current DB we'll restart from the next. This allows to
             * distribute the time evenly across DBs. */
            current_db++;
    
            /* Continue to expire if at the end of the cycle there are still
             * a big percentage of keys to expire, compared to the number of keys
             * we scanned. The percentage, stored in config_cycle_acceptable_stale
             * is not fixed, but depends on the Redis configured "expire effort". */
            do {
                unsigned long num, slots;
                long long now, ttl_sum;
                int ttl_samples;
                iteration++;
    
                /* If there is nothing to expire try next DB ASAP. */
                if ((num = dictSize(db->expires)) == 0) {
                    db->avg_ttl = 0;
                    break;
                }
                slots = dictSlots(db->expires);
                now = mstime();
    
                /* When there are less than 1% filled slots, sampling the key
                 * space is expensive, so stop here waiting for better times...
                 * The dictionary will be resized asap. */
                if (num && slots > DICT_HT_INITIAL_SIZE &&
                    (num*100/slots < 1)) break;
    
                /* The main collection cycle. Sample random keys among keys
                 * with an expire set, checking for expired ones. */
                expired = 0;
                sampled = 0;
                ttl_sum = 0;
                ttl_samples = 0;
    
                if (num > config_keys_per_loop)
                    num = config_keys_per_loop;
    
                /* Here we access the low level representation of the hash table
                 * for speed concerns: this makes this code coupled with dict.c,
                 * but it hardly changed in ten years.
                 *
                 * Note that certain places of the hash table may be empty,
                 * so we want also a stop condition about the number of
                 * buckets that we scanned. However scanning for free buckets
                 * is very fast: we are in the cache line scanning a sequential
                 * array of NULL pointers, so we can scan a lot more buckets
                 * than keys in the same time. */
                long max_buckets = num*20;
                long checked_buckets = 0;
    
                while (sampled < num && checked_buckets < max_buckets) {
                    for (int table = 0; table < 2; table++) {
                        if (table == 1 && !dictIsRehashing(db->expires)) break;
    
                        unsigned long idx = db->expires_cursor;
                        idx &= db->expires->ht[table].sizemask;
                        dictEntry *de = db->expires->ht[table].table[idx];
                        long long ttl;
    
                        /* Scan the current bucket of the current table. */
                        checked_buckets++;
                        while(de) {
                            /* Get the next entry now since this entry may get
                             * deleted. */
                            dictEntry *e = de;
                            de = de->next;
    
                            ttl = dictGetSignedIntegerVal(e)-now;
                            if (activeExpireCycleTryExpire(db,e,now)) expired++;
                            if (ttl > 0) {
                                /* We want the average TTL of keys yet
                                 * not expired. */
                                ttl_sum += ttl;
                                ttl_samples++;
                            }
                            sampled++;
                        }
                    }
                    db->expires_cursor++;
                }
                total_expired += expired;
                total_sampled += sampled;
    
                /* Update the average TTL stats for this database. */
                if (ttl_samples) {
                    long long avg_ttl = ttl_sum/ttl_samples;
    
                    /* Do a simple running average with a few samples.
                     * We just use the current estimate with a weight of 2%
                     * and the previous estimate with a weight of 98%. */
                    if (db->avg_ttl == 0) db->avg_ttl = avg_ttl;
                    db->avg_ttl = (db->avg_ttl/50)*49 + (avg_ttl/50);
                }
    
                /* We can't block forever here even if there are many keys to
                 * expire. So after a given amount of milliseconds return to the
                 * caller waiting for the other active expire cycle. */
                if ((iteration & 0xf) == 0) { /* check once every 16 iterations. */
                    elapsed = ustime()-start;
                    if (elapsed > timelimit) {
                        timelimit_exit = 1;
                        server.stat_expired_time_cap_reached_count++;
                        break;
                    }
                }
                /* We don't repeat the cycle for the current database if there are
                 * an acceptable amount of stale keys (logically expired but yet
                 * not reclaimed). */
            } while (sampled == 0 ||
                     (expired*100/sampled) > config_cycle_acceptable_stale);
        }
    }    
