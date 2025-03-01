#ifndef PIPE
#define PIPE

#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

template<typename T>
class Pipe {

	std::queue<T> queue_;
	std::mutex mutex_;
	std::condition_variable condVar_;
	size_t bufferSize;
	std::atomic<bool> done_;

public:
	Pipe() : bufferSize(2048), done_(false) {}
	~Pipe() { terminate(); }

	void terminate() {
		std::unique_lock<std::mutex> lock(mutex_);
		done_ = true;
		condVar_.notify_all();  // 通知所有等待的线程
	}

	bool done() const { return done_; }

	// 写者线程调用的函数
	void write(const T& data) {
		std::unique_lock<std::mutex> lock(mutex_);
		condVar_.wait(lock, [this] { return queue_.size() < bufferSize || done_; });

		if (!done_) {
			queue_.push(data);
			lock.unlock();  // 释放锁以便读者线程可以读取
			condVar_.notify_all();  // 通知读者线程
		}
	}

	// 读者线程调用的函数
	bool read(T& data) {
		std::unique_lock<std::mutex> lock(mutex_);
		if (queue_.empty() && !done_) {
			// 如果没有消息且写者未完成，则返回false
			return false;
		}

		if (!queue_.empty()) {
			data = queue_.front();
			queue_.pop();
		}

		lock.unlock();  // 释放锁以便写者线程可以写入
		condVar_.notify_all();  // 通知写者线程

		return true;
	}
};
#endif
