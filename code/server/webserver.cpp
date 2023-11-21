#include "webserver.h"

WebServer::WebServer(int port, int trigMode, int timeoutMS, bool OptLinger,
                     int sqlPort, const char* sqlUser, const char* sqlPwd,
                     const char* dbName, int connPoolNum, int threadNum,
                     bool openLog, int logLevel, int logQueSize)
    : port_(port),
      openLinger_(OptLinger),
      timeoutMS_(timeoutMS),
      isClose_(false),
      timer_(new HeapTimer()),
      threadpool_(new ThreadPool(threadNum)),
      epoller_(new Epoller()) {
  srcDir_ = getcwd(nullptr, 256);
  assert(srcDir_);
  strncat(srcDir_, "/resources/", 16);
  HttpConn::userCount = 0;
  HttpConn::srcDir = srcDir_;
  SqlConnPool::Instance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName,
                                connPoolNum);

  InitEventMode_(trigMode);
  if (!InitSocket_()) {
    isClose_ = true;
  }

  if (openLog) {
    Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
    if (isClose_) {
      LOG_ERROR("========== Server init error!==========");
    } else {
      LOG_INFO("========== Server init ==========");
      LOG_INFO("Port:%d, OpenLinger: %s", port_, OptLinger ? "true" : "false");
      LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
               (listenEvent_ & EPOLLET ? "ET" : "LT"),
               (connEvent_ & EPOLLET ? "ET" : "LT"));
      LOG_INFO("LogSys Level: %d", logLevel);
      LOG_INFO("srcDir: %s", HttpConn::srcDir);
      LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum,
               threadNum);
    }
  }
}

WebServer::~WebServer() {
  close(listenFd_);
  isClose_ = true;
  free(srcDir_);
  SqlConnPool::Instance()->ClosePool();
}

void WebServer::InitEventMode_(int trigMode) {
  listenEvent_ = EPOLLRDHUP;
  connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
  switch (trigMode) {
    case 0:
      break;
    case 1:
      connEvent_ |= EPOLLET;
      break;
    case 2:
      listenEvent_ |= EPOLLET;
      break;
    case 3:
      listenEvent_ |= EPOLLET;
      connEvent_ |= EPOLLET;
      break;
    default:
      listenEvent_ |= EPOLLET;
      connEvent_ |= EPOLLET;
      break;
  }
  HttpConn::isEt = (connEvent_ & EPOLLET);
}