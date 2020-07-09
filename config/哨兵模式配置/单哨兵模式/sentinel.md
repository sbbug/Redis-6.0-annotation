 
 #端口模式
 
 port 26379
 
 #添加守护进程模式
 
 daemonize yes

 #添加指明日志文件名

 logfile "/usr/local/redis/6379/sentinel26379.log"

 #修改工作目录
 
 dir "/usr/local/redis/6379"

 #修改启动端口
 
 port 26379

 #添加关闭保护模式
 
 protected-mode no

 #修改sentinel monitor
 
 sentinel monitor macrog-master 192.168.24.131 6379 1

 #将配置文件中mymaster全部替换macrog-master
 #在末行模式下 输入 :%s/mymaster/macrog-master/g

 依次修改26380,26381配置

 说明：
 macrog-master:监控主数据的名称,自定义即可,可以使用大小写字母和“.-_”符号
 192.168.24.131:监控的主数据库的IP
 6379:监控的主数据库的端口
 2:最低通过票数
