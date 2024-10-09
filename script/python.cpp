#include <pybind11/embed.h> // 包含pybind11嵌入头文件
#include <iostream>

namespace py = pybind11;

void execute_python_code() {
    // 初始化Python解释器
    py::scoped_interpreter guard{};

    try {
        // 执行Python代码
        py::exec(R"(
import sys
print("Python 版本:", sys.version)
)");
    } catch (const py::error_already_set& e) {
        std::cerr << "Python异常: " << e.what() << std::endl;
    }
}