 port 26379

 daemonize yes
 
 logfile "26379.log"
 
 dir "./"
 
 sentinel monitor mymaster 192.168.250.132 7000 2
 
 解释: 告诉sentinel去监听地址为ip:port的一个master，这里的master-name可以自定义，
 
 quorum是一个数字，指明当有多少个sentinel认为一个master失效时，master才算真正失效
 
 sentinel down-after-milliseconds mymaster 30000
 
 解释:这个配置项指定了需要多少失效时间，一个master才会被这个sentinel主观地认为是不可用的。 单位是毫秒，默认为30秒
 
 sentinel parallel-syncs mymaster 1
 
 sentinel failover-timeout mymaster 15000
 
 sentinel auth-pass mymaster 123
 
 bind 192.168.250.132 127.0.0.1