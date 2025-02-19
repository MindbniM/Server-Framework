# Server-Framework
本项目是对sylar服务器框架的学习并对其改编简化, 重点实现了日志模块, 配置模块, 协程模块, 协程调度模块, io协程调度模块, socket模块, TcpServer模块等  

**为了方便测试, 在测试的时候对一些问题代码做了修改, 并不适用全局, 正在将对一些模块进行重构, 修复之前的问题并使其更加灵活**
# 性能测试
服务器配置为2核2g
目前的测试只是单纯的对请求做响应
![1w连接, 1000并发](/bin/src/test1w.png "1w连接, 1000并发")
![1w连接, 1000并发](/bin/src/test10w.png "10w连接, 1w并发")

- 可以看出, 服务器在k级别的并发下表现良好
# 日志模块
- 支持流式日志风格写日志和格式化风格写日志
- 支持日志格式自定义，日志级别，多日志分离等等功能
- 支持时间,线程id,线程名称,日志级别,日志名称,文件名,行号等内容的自由配置
- 经测试, 在2核2g服务器上, 多线程并发向一个文件写日志, 可以达到20M/s以上
# 配置模块
- 使用YAML文件做为配置内容。支持级别格式的数据类型
- 支持STL容器(vector,list,set,map等等),支持自定义类型的支持（需要实现序列化和反序列化方法)
[示例](./tests/test_config.cc)
# 线程模块
- 封装了pthread里面的一些常用功能，Thread,Spinlock等，便于开发中对线程日常使用, 模拟thread构造设计
- 使用std::atomic_flag原子性标志实现自旋锁, 可自定义最大自旋次数
# 协程模块
- 协程：用户态的线程，相当于线程中的线程，更轻量级, 使用C++20协程封装
- 自定义awaiter, 后续封装io调用, 可使sleep, read等阻塞调用转换为异步操作
# 协程调度模块
- 协程调度器，管理协程的调度，内部实现为一个线程池，支持协程在多线程中切换，是一个N-M的协程调度模型，N个线程，M个协程。重复利用每一个线程。
- 任务队列空闲时, 协程会进入idle协程内忙等
# IO协程调度模块
- 继承与协程调度器，封装了epoll, 使得空闲线程进入idle等待事件就绪 
- 封装实现定时器功能, 支持Socket读写时间的添加，删除，取消功能。支持一次性定时器，循环定时器，条件定时器等功能
# Socket 模块
- 封装了Socket类，提供通用socket API功能
- 统一封装了Address类，将IPv4，IPv6，Unix地址统一起来。并且提供域名，IP解析功能
- 封装TcpSocket继承Socket, 细分功能
# TcpServer模块
- 增加缓冲区封装TcpConnect
- 基于TcpConnect类, 封装了一个通用的TcpServer的服务器类
- 可以快速绑定一个或多个地址，启动服务，监听端口，accept连接，处理socket连接等功能
- 具体业务功能更的服务器实现，只需要继承该类就可以快速实现

