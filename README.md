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

## 修改

本仓库基于markparticle大佬版本实现，并且修复了两个bug，具体见[issue96](https://github.com/markparticle/WebServer/issues/96)和[issue97](https://github.com/markparticle/WebServer/issues/97)：

1. httprequest.cpp：修复sql连接池重复添加的问题；
2. httprequest.cpp：修复加密字符解析错误的问题。