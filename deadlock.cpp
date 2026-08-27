#include <iostream>
#include <mutex>
#include <string>
#include <thread>

struct CriticalData {
    std::string name;
    std::mutex data_mtx;

    CriticalData(std::string name) : name(name) {}
};

void deadlock(CriticalData& data1, CriticalData& data2) {
    data1.data_mtx.lock();
    std::cout << "Thread: " << std::this_thread::get_id() << " get mutex of " << data1.name << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    data2.data_mtx.lock();
    std::cout << "Thread: " << std::this_thread::get_id() << " get mutex of " << data2.name << std::endl;

    data2.data_mtx.unlock();
    data1.data_mtx.unlock();
}

int main() {
    CriticalData data1("data1"), data2("data2");
    std::thread t1(deadlock, std::ref(data1), std::ref(data2));
    std::thread t2(deadlock, std::ref(data2), std::ref(data1));

    t1.join();
    t2.join();

    return 0;
}
