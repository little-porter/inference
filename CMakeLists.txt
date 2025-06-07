# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# 使用 set 命令手动设置项目版本号
set(PROJECT_VERSION "2.1.0")
set(APP_VERSION "2.1.0")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(Inference)

