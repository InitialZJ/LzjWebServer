#include "log.h"

Log::Log() {
  lineCount_ = 0;
  isAsync_ = false;
  writeThread_ = nullptr;
  deque_ = nullptr;
  toDay_ = 0;
  fp_ = nullptr;
}

Log::~Log() {
  if (writeThread_ && writeThread_->joinable()) {
    while (!deque_->empty()) {
      deque_->flush();
    }
    deque_->Close();
    writeThread_->join();
  }
  if (fp_) {
    std::lock_guard<std::mutex> locker(mtx_);
    flush();
    fclose(fp_);
  }
}

int Log::GetLevel() {
  std::lock_guard<std::mutex> locker(mtx_);
  return level_;
}

void Log::SetLevel(int level) {
  std::lock_guard<std::mutex> locker(mtx_);
  level_ = level;
}

void Log::init(int level = 1, const char* path, const char* suffix, int maxQueueSize) {
	isOpen_ = true;
	level_ = level;
	if (maxQueueSize > 0) {
		isAsync_ = true;
		if (!deque_) {
			std::unique_ptr<BlockDeque<std::string>> newDeque(new BlockDeque<std::string>);
			deque_ = std::move(newDeque);

			std::unique_ptr<std::thread> newThread(new std::thread(FlushLogThread));
			writeThread_ = std::move(newThread);
		}
	} else {
		
	}
}
