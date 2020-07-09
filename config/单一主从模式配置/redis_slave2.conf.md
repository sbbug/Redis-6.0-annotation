 # port 
 
 6381
 
 # AOF和快照文件文件夹
 
 dir /usr/local/var/db/redis_slave2/

 # 从节点要跟随的主节点
 
 slaveof 127.0.0.1 6379
 
 # 如果设置了密码，就要设置
 
 masterauth master-password