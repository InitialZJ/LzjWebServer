# LzjWebServer

Cpp后台必学



## 参考

qinguoyi大佬版本https://github.com/qinguoyi/TinyWebServer

markparticle大佬版本https://github.com/markparticle/WebServer

## 项目启动
需要先配置好对应的数据库
```bash
// 建立webserver库
create database webserver;

// 创建user表
USE webserver;
CREATE TABLE user(
    username char(50) NULL,
    password char(50) NULL
)ENGINE=InnoDB;

// 添加数据
INSERT INTO user(username, password) VALUES('name', 'password');
```

编译运行
```bash
make
./bin/server
```

## 压力测试

在关闭日志后，使用Webbench对服务器进行压力测试，对监听套接字（对应listenEvent_）和连接套接字（对应listenEvent_）分别采用ET和LT模式，均可实现上万的并发连接，下面列出的是两者组合后的测试结果：

LT+LT, 29037 QPS

![](https://raw.githubusercontent.com/InitialZJ/MarkdownPhotoes/main/res/lzjwebserver-webbench-lt-lt.png)

LT+ET, 50724 QPS

![](https://raw.githubusercontent.com/InitialZJ/MarkdownPhotoes/main/res/lzjwebserver-webbench-lt-et.png)

ET+LT, 29611 QPS

![](https://github.com/InitialZJ/MarkdownPhotoes/blob/main/res/lzjwebserver-webbench-et-lt.png?raw=true)

ET+ET, 29173 QPS

![](https://github.com/InitialZJ/MarkdownPhotoes/blob/main/res/lzjwebserver-webbench-et-et.png?raw=true)

## 修改

本仓库基于markparticle大佬版本实现，并且修复了两个bug，具体见[issue96](https://github.com/markparticle/WebServer/issues/96)和[issue97](https://github.com/markparticle/WebServer/issues/97)：

1. httprequest.cpp：修复sql连接池重复添加的问题；
2. httprequest.cpp：修复加密字符解析错误的问题。