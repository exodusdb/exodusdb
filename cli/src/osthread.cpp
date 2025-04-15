#include <queue>
#include <mutex>
#include <condition_variable>
//#include <thread>
#include <thread>
#include <optional>
#include <iostream>
#include <string>

template<typename T>
class OSThread {
public:
    enum class Status { SUCCESS, EMPTY, STOPPED };

    OSThread() : running_(true), worker_(&OSThread::run, this) {}
    virtual ~OSThread() {
        std::lock_guard<std::mutex> in_lock(in_mutex_);
        std::lock_guard<std::mutex> out_lock(out_mutex_);
        running_ = false;
        in_cv_.notify_one();
        out_cv_.notify_one();
        if (worker_.joinable()) worker_.join();
    }

    Status write_input(const T& data) {
        std::lock_guard<std::mutex> lock(in_mutex_);
        if (!running_) return Status::STOPPED;
        in_queue_.push(data);
        in_cv_.notify_one();
        return Status::SUCCESS;
    }

    std::pair<Status, std::optional<T>> read_output() {
        std::unique_lock<std::mutex> lock(out_mutex_);
        if (!running_ && out_queue_.empty()) return {Status::STOPPED, std::nullopt};
        if (out_queue_.empty()) return {Status::EMPTY, std::nullopt};
        T data = out_queue_.front();
        out_queue_.pop();
        return {Status::SUCCESS, data};
    }

protected:
    std::optional<T> read_input() {
        std::unique_lock<std::mutex> lock(in_mutex_);
        in_cv_.wait(lock, [this] { return !in_queue_.empty() || !running_; });
        if (!running_ && in_queue_.empty()) return std::nullopt;
        T data = in_queue_.front();
        in_queue_.pop();
        return data;
    }

    void write_output(const T& data) {
        std::lock_guard<std::mutex> lock(out_mutex_);
        out_queue_.push(data);
        out_cv_.notify_one();
    }

    virtual void process() = 0;

private:
    std::queue<T> in_queue_;
    std::queue<T> out_queue_;
    std::mutex in_mutex_;
    std::mutex out_mutex_;
    std::condition_variable in_cv_;
    std::condition_variable out_cv_;
    bool running_;
    std::thread worker_;

    void run() {
        process();
    }
};

// String-based thread that echoes messages
class MessageEchoThread : public OSThread<std::string> {
protected:
    void process() override {
        while (true) {
			std::clog << "MessageEchoThread input " << std::endl;
            std::optional<std::string> msg = read_input();
            if (!msg) break;
            write_output("ECHO: " + *msg); // Add a prefix for fun
        }
		std::clog << "MessageEchoThread exit" << std::endl;
    }
};

// String-based thread that modifies messages
class MessageUpperThread : public OSThread<std::string> {
protected:
    void process() override {
        while (true) {
			std::clog << "MessageUpperThread input " << std::endl;
            std::optional<std::string> msg = read_input();
            if (!msg) break;
            std::string upper = *msg;
            for (char& c : upper) c = std::toupper(c);
            write_output(upper);
        }
		std::clog << "MessageUpperThread exit" << std::endl;
    }
};

int main() {
    MessageEchoThread echo;
    MessageUpperThread upper;

    // Test MessageEchoThread
    echo.write_input("Hello, world!");
    echo.write_input("xxxx");
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Let thread process
    auto [status1, value1] = echo.read_output();
    if (status1 == OSThread<std::string>::Status::SUCCESS) {
        std::cout << "Echo output: " << *value1 << "\n"; // Prints "ECHO: Hello, world!"
    }
    echo.write_input("yyyyyyyyyyyyyyyyyyyyyyyy");

    // Test MessageUpperThread
    upper.write_input("make me loud");
    upper.write_input("xxxxxxxxxxxxxxxxxxx");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto [status2, value2] = upper.read_output();
    if (status2 == OSThread<std::string>::Status::SUCCESS) {
        std::cout << "Upper output: " << *value2 << "\n"; // Prints "MAKE ME LOUD"
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Let thread process
    return 0;
}