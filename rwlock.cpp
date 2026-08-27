#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>

class A {
  public:
    int read() const {
        std::shared_lock<std::shared_mutex> l(shared_mtx_);
        return n_;
    }

    int write() {
        std::unique_lock<std::shared_mutex> l(shared_mtx_);
        return ++n_;
    }

    int getN() const { return n_; }

  private:
    mutable std::shared_mutex shared_mtx_;
    int n_{0};
};

int main() {
    const int N = 100;
    A a;
    std::vector<std::thread> read_threads;
    std::vector<std::thread> write_threads;
    for (int i = 0; i < N; ++i) {
        read_threads.emplace_back([&a]() { a.read(); });
        write_threads.emplace_back([&a]() { a.write(); });
    }
    for (int i = 0; i < N; ++i) {
        read_threads[i].join();
        write_threads[i].join();
    }

    std::cout << a.getN() << std::endl;
    return 0;
}
