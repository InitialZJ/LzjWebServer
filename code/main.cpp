#include <unistd.h>

#include "server/webserver.h"

int main() {
  WebServer server(1319,                    // 端口号
                   3,                       // 触发模式
                   60000,                   // 多少毫秒自动关闭连接
                   false,                   // 是否支持优雅关闭
                   3306, "root", "123456",  // 数据库端口号、用户名、密码
                   "webserver",             // 数据库名
                   8,                       // 连接池数量
                   10,                      // 线程池数量
                   true,                    // 日志开关
                   1,                       // 日志等级
                   1024                     // 日志异步队列容量
  );
  server.Start();
  return 0;
}
