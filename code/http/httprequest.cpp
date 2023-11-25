#include "httprequest.h"

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML{
    "/index", "/register", "/login", "welcome", "/video", "/picture",
};

const std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG{
    {"/register.html", 0},
    {"/login.html", 1},
};

void HttpRequest::Init() {
  method_ = path_ = version_ = body_ = "";
  state_ = REQUEST_LINE;
  header_.clear();
  post_.clear();
}

bool HttpRequest::IsKeepAlive() const {
  if (header_.find("Connection") != header_.end()) {
    return header_.at("Connection") == "keep-alive" && version_ == "1.1";
  }
  return false;
}

// GET / HTTP/1.1
// Host: localhost:1316
// User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:109.0) Gecko/20100101
// Firefox/119.0 Accept:
// text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8
// Accept-Language: zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2
// Accept-Encoding: gzip, deflate, br
// Connection: keep-alive
// Upgrade-Insecure-Requests: 1
// Sec-Fetch-Dest: document
// Sec-Fetch-Mode: navigate
// Sec-Fetch-Site: none
// Sec-Fetch-User: ?1

// POST /login HTTP/1.1
// Host: localhost:1316
// User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:109.0) Gecko/20100101
// Firefox/119.0 Accept:
// text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8
// Accept-Language: zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2
// Accept-Encoding: gzip, deflate, br
// Content-Type: application/x-www-form-urlencoded
// Content-Length: 21
// Origin: null
// Connection: keep-alive
// Upgrade-Insecure-Requests: 1
// Sec-Fetch-Dest: document
// Sec-Fetch-Mode: navigate
// Sec-Fetch-Site: cross-site

// username=1&password=1

bool HttpRequest::parse(Buffer& buff) {
  const char CRLF[] = "\r\n";
  if (buff.ReadableBytes() <= 0) {
    return false;
  }
  while (buff.ReadableBytes() && state_ != FINISH) {
    const char* lineEnd =
        std::search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF + 2);
    std::string line(buff.Peek(), lineEnd);
    switch (state_) {
      case REQUEST_LINE:
        if (!ParseRequestLine_(line)) {
          return false;
        }
        ParsePath_();
        break;
      case HEADERS:
        ParseHeader_(line);
        if (buff.ReadableBytes() <= 2) {
          state_ = FINISH;
        }
        break;
      case BODY:
        ParseBody_(line);
        break;
      default:
        break;
    }
    if (lineEnd == buff.BeginWrite()) {
      buff.RetrieveUntil(lineEnd);
      break;
    }
    buff.RetrieveUntil(lineEnd + 2);
  }
  LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(),
            version_.c_str());
  return true;
}

void HttpRequest::ParsePath_() {
  if (path_ == "/") {
    path_ = "/index.html";
  } else if (DEFAULT_HTML.find(path_) != DEFAULT_HTML.end()) {
    path_ += ".html";
  }
}

bool HttpRequest::ParseRequestLine_(const std::string& line) {
  std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
  std::smatch subMatch;
  if (std::regex_match(line, subMatch, patten)) {
    method_ = subMatch[1];
    path_ = subMatch[2];
    version_ = subMatch[3];
    state_ = HEADERS;
    return true;
  }
  LOG_ERROR("RequestLine Error");
  return false;
}

void HttpRequest::ParseHeader_(const std::string& line) {
  std::regex patten("^([^:]*): ?(.*)$");
  std::smatch subMatch;
  if (std::regex_match(line, subMatch, patten)) {
    header_[subMatch[1]] = subMatch[2];
  } else {
    state_ = BODY;
  }
}

void HttpRequest::ParseBody_(const std::string& line) {
  body_ = line;
  ParsePost_();
  state_ = FINISH;
  LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

int HttpRequest::ConverHex(char ch) {
  unsigned int y;
  if (ch >= 'A' && ch <= 'F') {
    y = ch - 'A' + 10;
  } else if (ch >= 'a' && ch <= 'f') {
    y = ch - 'a' + 10;
  } else if (ch >= '0' && ch <= '9') {
    y = ch - '0';
  } else {
    assert(0);
  }
  return y;
}

void HttpRequest::ParsePost_() {
  if (method_ == "POST" &&
      header_["Content-Type"] == "application/x-www-form-urlencoded") {
    ParseFromUrlencoded_();
    if (DEFAULT_HTML_TAG.find(path_) != DEFAULT_HTML_TAG.end()) {
      int tag = DEFAULT_HTML_TAG.at(path_);
      LOG_DEBUG("Tag:%d", tag);
      if (tag == 0 || tag == 1) {
        bool isLogin = (tag == 1);
        if (UserVerify(post_["username"], post_["password"], isLogin)) {
          path_ = "/welcome.html";
        } else {
          path_ = "/error.html";
        }
      }
    }
  }
}

void HttpRequest::ParseFromUrlencoded_() {
  if (body_.size() == 0) {
    return;
  }

  std::string key, value;
  int num = 0;
  int n = body_.size();
  int i = 0, j = 0;

  for (; i < n; i++) {
    char ch = body_[i];
    switch (ch) {
      case '=':
        key = body_.substr(j, i - j);
        j = i + 1;
        break;
      case '+':
        body_[i] = ' ';
        break;
      case '%':
        num = ConverHex(body_[i + 1]) * 16 + ConverHex(body_[i + 2]);
        body_[i] = num;
        body_.erase(body_.begin() + i + 1);
        body_.erase(body_.begin() + i + 1);
        n -= 2;
        break;
      case '&':
        value = body_.substr(j, i - j);
        j = i + 1;
        post_[key] = value;
        LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
        break;
      default:
        break;
    }
  }
  assert(j <= i);
  if (post_.find(key) == post_.end() && j < i) {
    value = body_.substr(j, i - j);
    post_[key] = value;
  }
}

bool HttpRequest::UserVerify(const std::string& name, const std::string& pwd,
                             bool isLogin) {
  if (name == "" || pwd == "") {
    return false;
  }
  LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());
  MYSQL* sql;
  SqlConnRAII(&sql, SqlConnPool::Instance());
  assert(sql);

  bool flag = !isLogin;
  char order[256] = {0};
  MYSQL_RES* res = nullptr;

  // 查询用户及密码
  snprintf(order, 256,
           "SELECT username, password FROM user WHERE username='%s' LIMIT 1",
           name.c_str());
  LOG_DEBUG("%s", order);

  if (mysql_query(sql, order)) {
    mysql_free_result(res);
    return false;
  }
  res = mysql_store_result(sql);

  while (MYSQL_ROW row = mysql_fetch_row(res)) {
    LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
    std::string password(row[1]);
    if (isLogin) {
      // 登录行为
      if (pwd == password) {
        flag = true;
      } else {
        flag = false;
        LOG_ERROR("pwd error!");
      }
    } else {
      // 注册行为，且用户名已被使用
      flag = false;
      LOG_DEBUG("user used!");
    }
  }
  mysql_free_result(res);

  // 注册行为，且用户名未被使用
  if (!isLogin && flag == true) {
    LOG_DEBUG("register!");
    bzero(order, 256);
    snprintf(order, 256,
             "INSERT INTO user(username, password) VALUES('%s', '%s')",
             name.c_str(), pwd.c_str());
    LOG_DEBUG("%s", order);
    if (mysql_query(sql, order)) {
      LOG_DEBUG("Insert error!");
      flag = false;
    }
    flag = true;
  }
  // TODO: 以下这句话需要吗
  // SqlConnPool::Instance()->FreeConn(sql);
  LOG_DEBUG("UserVerify success!!");
  if (flag) {
    LOG_DEBUG("Verification passed");
  } else {
    LOG_DEBUG("Verification not passed");
  }
  return flag;
}

std::string HttpRequest::path() const { return path_; }

std::string& HttpRequest::path() { return path_; }

std::string HttpRequest::method() const { return method_; }

std::string HttpRequest::version() const { return version_; }

std::string HttpRequest::GetPost(const std::string& key) const {
  assert(key != "");
  if (post_.find(key) != post_.end()) {
    return post_.at(key);
  }
  return "";
}

std::string HttpRequest::GetPost(const char* key) const {
  assert(key != nullptr);
  if (post_.find(key) != post_.end()) {
    return post_.at(key);
  }
  return "";
}
