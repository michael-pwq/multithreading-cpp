#include <future>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

std::mutex cout_mtx;

struct Divide {
    void operator()(std::promise<int>&& promise, int a, int b) const {
        try {
            if (b == 0) throw std::runtime_error("Illegal division by zero!");
            promise.set_value(a / b);
        } catch (...) { promise.set_exception(std::current_exception()); }
    }
};

struct Requestor {
    void operator()(std::shared_future<int> shared_fut) {
        std::lock_guard<std::mutex> l(cout_mtx);
        std::cout << "thread id(" << std::this_thread::get_id() << "): ";
        try {
            std::cout << shared_fut.get() << std::endl;
        } catch (const std::exception& e) { std::cout << e.what() << std::endl; }
    }
};

int main() {
    // define the promise
    std::promise<int> div_promise;

    // get the shared future
    std::shared_future<int> shared_fut = div_promise.get_future();

    int a = 20, b = 10;
    // calculate in a separate thread
    Divide divide;
    std::thread div_thread(divide, std::move(div_promise), a, b);

    Requestor requestor;
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) threads.emplace_back(requestor, shared_fut);

    div_thread.join();
    for (auto& t : threads) t.join();

    return 0;
}
