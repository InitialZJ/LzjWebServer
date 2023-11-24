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

```bash
make
./bin/server
```