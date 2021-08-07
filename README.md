# **My Poor Web Srever**


# 概述

此项目基本模型属于的是B/S模型；（Browser-Server Model)

## 服务器编程框架

<div align=center><img src="https://wx1.sinaimg.cn/mw2000/006UYdYPly1gt8dgz48ybj30ry0d475m.jpg" width="700"></div>

### IO 处理单元

* **非阻塞socket** 
  
  socket 在创建的时候默认是阻塞的，设置为非阻塞才能发挥 epoll 的作用；

* **事件处理模式**
  - 三类事件：IO事件、定时事件、信号
  
  事件处理模式有：Reactor、Proactor;

  同步IO通常用于实现 Reactor；而异步IO通常用于实现 Proactor;本项目中的 Proactor 则是由同步IO模拟实现的；

  **Reactor** 主线程（IO处理单元）只负责监听文件描述符是否有事件发生，有的话则通知工作线程（逻辑单元，线程池中的线程），主线程没有作任何其他实质性工作；读写数据、接受新连接、处理客户请求均在工作线程中完成；因此主线程往 epoll 内核事件表中注册的是 cocket 上的**就绪事件**；

  <div align=center><img src="https://wx1.sinaimg.cn/mw2000/006UYdYPly1gt8dgw53qgj30yz0b8taq.jpg" width="700"></div>

  **Proactor** IO 操作都交给主线程完成，工作线程只负责业务逻辑处理（即本项目中的报文解析和生成应答报文），因此主线程往 epoll内核事件表中注册的是 **读完成事件**，工作线程业务处理完成后往 epoll 内核事件表注册 **写完成事件**；

  <div align=center><img src="https://wx1.sinaimg.cn/mw2000/006UYdYPly1gt8dgvzdbuj310f0agdi7.jpg" width="700"></div>

  **模拟 Proactor** 
  <div align=center><img src="https://wx1.sinaimg.cn/mw2000/006UYdYPly1gt8dgzbvahj60zo0mtjux02.jpg" width="700"></div>



* **并发模型**
  
  并发的目的是让程序同时执行多个任务；

  并发模式是指IO处理单元和多个逻辑单元之间协调完成任务的方法；

  - 并发编程模式：半同步/半异步模式、领导者/追随者模式；本项目使用的就是半同步/半异步模式的优化版本 半同步/半反应堆模式；
  
> IO 同步/异步 与 并发模式中的同步/异步 ？？

> 半同步/半异步：同步指程序完全按照代码顺序来执行；异步是指程序的执行由系统事件来驱动；（中断、信号等）<br>
> IO 同步/异步：指区分内核向应用程序通知的是哪种IO事件（就绪还是完成事件），以及该由谁来完成IO读写（应用程序还是内核）

* 半同步/半反应堆

  - 同步线程简单，能同时处理多个请求；异步线程，实时性强，执行效率高；—— 工作线程
  - 同步线程用于处理客户逻辑；异步线程用于处理IO事件；—— 主线程
  
  <div align=center><img src="https://wx1.sinaimg.cn/mw2000/006UYdYPly1gt8dgytlojj30sg0d8wgq.jpg" width="700"></div>

* 领导者/追随者模式
  - 也就是不停的换领导者，先推选新的领导者，自己去处理接受的IO读写和逻辑，新的领导者继续监听新的连接；缺点就是仅支持一个事件源集合；

### 逻辑处理
* 有限状态机
  - 主从状态机应用：HTTP 报文的读取与解析

  <div align=center><img src="https://wx1.sinaimg.cn/mw2000/006UYdYPly1gt8dgzi8ywj30u00dugto.jpg" width="700"></div>
  <center>——— 图片来源见水印 ———</center>

  主状态机三个状态：CHECK_STATE_REQUESTLINE（请求行）CHECK_STATE_HEADER（请求头）CHECK_STATE_CONTENT（消息体 - 仅用于 POST）

  <div align=center><img src="https://wx1.sinaimg.cn/mw2000/006UYdYPly1gt8dgyz6ogj30il0fn0up.jpg" width="700"></div>

  从状态机三个状态：LINE_OK,LINE_OPEN,LINE_BAD

### 线程池、连接池

线程池：预先存放一定数量的线程，请求来了直接用，避免使用时才构造带来的较大开销；
连接池：一样的目的，作用是用于连接数据库，查询数据库中中存放的用户数据库（包含用户名称和密码）

### 锁

在 游双《Linux高性能服务器》书中，提供了详细的代码及说明；

<div align="center"> <img src="https://wx1.sinaimg.cn/mw2000/006UYdYPly1gt8dgvpfxkj30ra0zhk0x.jpg" width="200"/> </div><br>

* 三种锁：信号量、互斥锁、条件变量
  - 半同步/半反应堆：信号量或者条件变量
  - 互斥锁：更改资源时就需要加互斥锁

### 定时器

定时器实现有：升序链表定时器，时间轮定时器，时间堆定时器；此项目利用时间轮定时器关闭非活动连接；

升序链表：添加定时器的事件复杂度O(n)，删除操作O(1)，执行任务O(1);

时间轮：添加O(1)，删除O(1)，执行O(n) 实际效率比 O(n) 效果好；使用多轮时效率也越接近O(1);

时间堆：添加O(logn)，删除O(1)，执行O(1);

# Demo

<div align="center"> <img src="https://wx1.sinaimg.cn/large/006UYdYPly1gt8em4jwzmg31gw0m0qv7.gif" width="600"/> </div><br>

# 压测

实现近10K的并发量；

<div align="center"> <img src="https://wx1.sinaimg.cn/mw2000/006UYdYPly1gt8dgv9oxkj30q00700xc.jpg" width="600"/> </div><br>

# 下一步

- [x] 实现文件上传或者下载功能；

# 测试
## 环境

MySQL & ubuntu:

`mysql  Ver 8.0.25-0ubuntu0.20.04.1 for Linux on x86_64 ((Ubuntu))`

Browser:

`windows (MS Edge) & linux (Firefox) 均测试通过` 

## 运行
* 测试前确认已安装MySQL数据库

    ```C++
    // 建立yourdb库
    create database 你的数据库名称;

    // 创建user表
    USE 你的数据库名称;
    CREATE TABLE user(
        username char(50) NULL,
        passwd char(50) NULL
    )ENGINE=InnoDB;

    // 添加数据
    INSERT INTO user(username, passwd) VALUES('name', 'passwd');
    ```

* 修改main.cpp中的数据库初始化信息

    ```C++
    //数据库登录名,密码,库名
    string user = "你的mysql用户名";  
    string passwd = "密码";
    string databasename = "你的数据库名称"; 
    ```

* build

    ```C++
    sh ./build.sh
    ```

* 启动server

    ```C++
    ./server
    ```

# 致谢

书籍《Linux高性能服务器编程》- 游双著；

<div align="center"> <img src="https://wx1.sinaimg.cn/mw2000/006UYdYPly1gt8dgvpfxkj30ra0zhk0x.jpg" width="200"/> </div><br>

Github作者： 在此大佬代码基础上修改[@qinguoyi](https://github.com/qinguoyi)
