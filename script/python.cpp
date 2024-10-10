#include <pybind11/embed.h> // 包含pybind11嵌入头文件
#include <pybind11/stl.h> // 包含pybind11 STL头文件
#include <iostream>
#include <vector>

namespace py = pybind11;

// 初始化Python解释器
py::scoped_interpreter guard{};

py::object call_python_function(const std::string& module_name, const std::string& function_name, py::args args) {
    try {
        // 导入Python模块
        py::module_ module = py::module_::import(module_name.c_str());
        
        // 获取指定的函数
        py::object func = module.attr(function_name.c_str());
        
        // 调用函数并返回结果
        return func(*args);
    } catch (const py::error_already_set& e) {
        std::cerr << "Python异常: " << e.what() << std::endl;
        return py::none();
    }
}

// 示例：调用Python函数并处理返回值
void example_python_call() {
    // 调用Python的math模块中的pow函数
    auto result = call_python_function("math", "pow", py::make_tuple(2, 3));
    if (!result.is_none()) {
        double power = result.cast<double>();
        std::cout << "2^3 = " << power << std::endl;
    }

    // 调用自定义Python函数
    auto custom_result = call_python_function("my_module", "my_function", py::make_tuple(10, 20));
    if (!custom_result.is_none()) {
        int sum = custom_result.cast<int>();
        std::cout << "自定义函数结果: " << sum << std::endl;
    }
}