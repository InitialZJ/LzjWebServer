#include "httprequest.h"

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML{
    "/index", "register", "/login", "welcome", "/video", "/picture",
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
}

// GET / HTTP/1.1
// Accept:
// text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
// Accept-Encoding: gzip, deflate, br
// Accept-Language: zh-CN,zh;q=0.9
// Cache-Control: max-age=0
// Connection: keep-alive
// Host: localhost:1316
// Sec-Fetch-Dest: document
// Sec-Fetch-Mode: navigate
// Sec-Fetch-Site: none
// Sec-Fetch-User: ?1
// Upgrade-Insecure-Requests: 1
// User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like
// Gecko) Chrome/119.0.0.0 Safari/537.36 sec-ch-ua: "Google Chrome";v="119",
// "Chromium";v="119", "Not?A_Brand";v="24" sec-ch-ua-mobile: ?0
// sec-ch-ua-platform: "Linux"

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
			break;
		}
		buff.RetrieveUntil(lineEnd + 2);
  }
	LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
	return true;
}

void HttpRequest::ParsePath_() {
	if (path_ == "/") {
		path_ = "/index.html";
	}
}
