#include <chrono>
#include <future>
#include <iostream>
#include <thread>

int main() {
    auto begin = std::chrono::system_clock::now();

    auto async_lazy = std::async(std::launch::deferred, [] { return std::chrono::system_clock::now(); });

    auto async_eager = std::async(std::launch::async, [] { return std::chrono::system_clock::now(); });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    auto lazyStart = async_lazy.get() - begin;
    auto eagerStart = async_eager.get() - begin;

    auto lazy_dur = std::chrono::duration<double>(lazyStart).count();
    auto eager_dur = std::chrono::duration<double>(eagerStart).count();

    std::cout << "async_lazy evaluated after: " << lazy_dur << " seconds." << std::endl;
    std::cout << "async_eager evaluated after: " << eager_dur << " seconds." << std::endl;

    return 0;
}
