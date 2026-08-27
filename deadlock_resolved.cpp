#include <iostream>
#include <mutex>
#include <string>
#include <thread>

struct CriticalData {
    std::string name;
    std::mutex data_mtx;

    CriticalData(std::string name) : name(name) {}
};

void deadlock_resolved(CriticalData& data1, CriticalData& data2) {
    std::unique_lock<std::mutex> lock1(data1.data_mtx, std::defer_lock);
    std::cout << "Thread: " << std::this_thread::get_id() << " mutex of " << data1.name << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    std::unique_lock<std::mutex> lock2(data2.data_mtx, std::defer_lock);
    std::cout << "Thread: " << std::this_thread::get_id() << " mutex of " << data2.name << std::endl;

    // lock here
    std::lock(lock1, lock2);
    std::cout << "Thread: " << std::this_thread::get_id() << " get both mutex" << std::endl;
    // do some work
}

int main() {
    CriticalData data1("data1"), data2("data2");
    std::thread t1(deadlock_resolved, std::ref(data1), std::ref(data2));
    std::thread t2(deadlock_resolved, std::ref(data2), std::ref(data1));

    t1.join();
    t2.join();

    return 0;
}
