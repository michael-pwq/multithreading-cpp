#include <future>
#include <iostream>
#include <string>

std::string hello_func(const std::string& s) { return "Hello C++11 from " + s + "."; }

class HelloFuncObject {
  public:
    std::string operator()(const std::string& s) const { return "Hello C++11 from " + s + "."; }
};

int main() {
    // future with function
    auto fut_func = std::async(hello_func, "function");

    // future with function object
    HelloFuncObject hello_fun_obj;
    auto fut_func_obj = std::async(hello_fun_obj, "function object");

    // future with lambda function
    auto fut_lambda = std::async([](const std::string& s) { return "Hello C++11 from " + s + "."; }, "lambda function");

    std::cout << fut_func.get() << "\n" << fut_func_obj.get() << "\n" << fut_lambda.get() << std::endl;

    return 0;
}
