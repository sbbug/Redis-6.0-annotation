

## redis 6.0引入了多线程机制

    Redis默认并没有开启多线程机制。
    在conf文件进行配置
    io-threads-do-reads yes
    io-threads 线程数
    官方建议：4 核的机器建议设置为 2 或 3 个线程，8 核的建议设置为 6 个线程，线程数一定要小于机器核数，尽量不超过8个。 