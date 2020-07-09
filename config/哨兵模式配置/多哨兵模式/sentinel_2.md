 port 26380

 daemonize yes
 
 logfile "26380.log"
 
 dir "./"
 
 sentinel monitor mymaster 192.168.250.132 7000 2
 
 sentinel down-after-milliseconds mymaster 30000
 
 sentinel parallel-syncs mymaster 1
 
 sentinel failover-timeout mymaster 15000
 
 sentinel auth-pass mymaster 123
 
 bind 192.168.250.132 127.0.0.1