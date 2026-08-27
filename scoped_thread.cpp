#include <iostream>
#include <stdexcept>
#include <thread>
#include <utility>

class ScopedThread {
  public:
    explicit ScopedThread(std::thread t) : t_(std::move(t)) {
        if (!t_.joinable()) throw std::logic_error("No thread");
    }
    ~ScopedThread() { t_.join(); }
    ScopedThread(ScopedThread&) = delete;
    ScopedThread& operator=(ScopedThread const&) = delete;

  private:
    std::thread t_;
};

int main() {
    ScopedThread st(std::thread([] { std::cout << std::this_thread::get_id() << std::endl; }));
    return 0;
}
