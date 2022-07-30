## Linux下redis源码编译安装流程:

    1、下载源码并解压
        wget url
        tar -xvf ***.tar.gz
        cd 根目录
    2、源码编译
        主要实现将Redis源码编译成.o文件
       make
    3、安装
        make PREFIX=/usr/local/redis install //prefix指定安装目录
        mkdir /usr/local/redis/etc/          //创建文件配置目录
        cp redis.conf /usr/local/redis/etc/  //拷贝配置文件到指定目录
        cd /usr/local/redis/bin/             //进入redis bin目录
        cp redis-benchmark redis-cli redis-server /usr/bin/ //将相关启动文件拷贝到系统目录bin
    4、修改/usr/local/redis/etc/redis.conf配置文件
    5、配置环境变量
        vim /etc/profile  //全局环境变量
        export PATH="$PATH:/usr/local/redis/bin"
        source /etc/profile
    6、配置Redis启动脚本，命名为redis
        #!/bin/bash
        #chkconfig: 2345 80 90
        # Simple Redis init.d script conceived to work on Linux systems
        # as it does use of the /proc filesystem.
        
        PATH=/usr/local/bin:/sbin:/usr/bin:/bin
        REDISPORT=6379
        EXEC=/usr/local/redis/bin/redis-server
        REDIS_CLI=/usr/local/redis/bin/redis-cli
           
        PIDFILE=/var/run/redis.pid
        CONF="/usr/local/redis/etc/redis.conf"
           
        case "$1" in # $1运行该脚本时bai的第一个命令行参数
            start)
                if [ -f $PIDFILE ] #如果进程ID存在，说明已经在运行。-f 判断是不是文件
                then
                        echo "$PIDFILE exists, process is already running or crashed"
                else
                        echo "Starting Redis server..."
                        $EXEC $CONF #启动服务
                fi
                if [ "$?"="0" ] #$?代表上一个程序执行是否成功，如果成功则为0
                then
                      echo "Redis is running..."
                fi
                ;;
            stop)
                if [ ! -f $PIDFILE ] # pid文件并不存在，不需要停止
                then
                        echo "$PIDFILE does not exist, process is not running"
                else
                        PID=$(cat $PIDFILE) #获取pid进程编号
                        echo "Stopping ..."
                        $REDIS_CLI -p $REDISPORT SHUTDOWN #关闭服务
                        while [ -x ${PIDFILE} ]
                       do
                            echo "Waiting for Redis to shutdown ..."
                            sleep 1
                       done
                       echo "Redis stopped"
                fi
                ;;
           restart|force-reload)
                ${0} stop
                ${0} start
                ;;
          *)
            echo "Usage: /etc/init.d/redis {start|stop|restart|force-reload}" >&2
                exit 1
        esac
    7、开机自启动设置
        # 复制脚本文件到init.d目录下
        cp redis /etc/init.d/
        
        # 给脚本增加运行权限
        chmod +x /etc/init.d/redis
        
        # 查看服务列表
        chkconfig --list
        
        # 添加服务
        chkconfig --add redis
        
        # 配置启动级别
        chkconfig --level 2345 redis on
        
    8、启动测试
        systemctl start redis   #或者 /etc/init.d/redis start  
        systemctl stop redis   #或者 /etc/init.d/redis stop
        
        # 查看redis进程
        ps -el|grep redis
        
        # 端口查看
        netstat -an|grep 6379

## 关于源码编译的Makefile

    Makefile是类似shell脚本，可以执行操作系统命令。
    一个C++/C语言项目工程非常庞大，有的文件需要先编译，有的需要后编译，有的需要重复编译，有的需要指定版本才可以使用。
    这些约束规则都是通过Makefile文件实现。

## ASCII与unicode(UTF-8)
    
   ascii是美国标准信息交换码，128个英文字符，分别用一个字节就可以表示。满足英语语言的计算表示和存储。
   但各国语言五花八门，只用ascii编码不能满足。
   因此出现了unicode编码集合，其中UTF-8可以有效解决各种语言的编码问题。
