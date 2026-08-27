#include <iostream>
#include <thread>

void func(int x) {
    std::cout << "Hey, I'm thread " << std::this_thread::get_id() << std::endl;
    std::cout << "Argument passed in: " << x << std::endl;
}

class FuncObject {
  public:
    void operator()(int x) const {
        std::cout << "Hey, I'm thread " << std::this_thread::get_id() << std::endl;
        std::cout << "Argument passed in: " << x << std::endl;
    }
};

int main() {
    auto lambda_func = [](int x) {
        std::cout << "Hey, I'm thread " << std::this_thread::get_id() << std::endl;
        std::cout << "Argument passed in: " << x << std::endl;
    };

    std::thread t1(func, 1);
    std::thread t2(lambda_func, 2);
    FuncObject func_obj;
    std::thread t3(func_obj, 3);
    std::thread t4(
        [](int x) {
            std::cout << "Hey, I'm thread " << std::this_thread::get_id() << std::endl;
            std::cout << "Argument passed in: " << x << std::endl;
        },
        4);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    std::cout << "Hello from main thread!" << std::endl;

    return 0;
}
