#include <future>
#include <iostream>
#include <thread>

void product(std::promise<int>&& promise, int a, int b) { promise.set_value(a * b); }

struct Divide {
    void operator()(std::promise<int>&& promise, int a, int b) const { promise.set_value(a / b); }
};

int main() {
    // define the promises
    std::promise<int> prod_promise;
    std::promise<int> div_promise;

    // get the futures
    std::future<int> prod_fut = prod_promise.get_future();
    std::future<int> div_fut = div_promise.get_future();

    int a = 20, b = 10;
    // calculate concurrently
    std::thread prod_thread(product, std::move(prod_promise), a, b);
    Divide divide;
    std::thread div_thread(divide, std::move(div_promise), a, b);

    // get the results
    std::cout << a << "*" << b << "=" << prod_fut.get() << std::endl;
    std::cout << a << "/" << b << "=" << div_fut.get() << std::endl;

    prod_thread.join();
    div_thread.join();

    return 0;
}
