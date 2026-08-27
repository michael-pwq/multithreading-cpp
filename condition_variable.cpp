#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

class A {
  public:
    void step1() {
        {
            std::lock_guard<std::mutex> l(m_);
            step1_done_ = true;
        }
        std::cout << "step1 done" << std::endl;
        cv_.notify_one();
    }

    void step2() {
        std::unique_lock<std::mutex> l(m_);
        cv_.wait(l, [this] { return step1_done_; });
        step2_done_ = true;
        std::cout << "step2 done" << std::endl;
        cv_.notify_one();
    }

    void step3() {
        std::unique_lock<std::mutex> l(m_);
        cv_.wait(l, [this] { return step2_done_; });
        std::cout << "step3 done" << std::endl;
    }

  private:
    std::mutex m_;
    std::condition_variable cv_;
    bool step1_done_ = false;
    bool step2_done_ = false;
};

int main() {
    A a;
    std::thread t1(&A::step1, &a);
    std::thread t2(&A::step2, &a);
    std::thread t3(&A::step3, &a);
    t1.join();
    t2.join();
    t3.join();

    return 0;
}
