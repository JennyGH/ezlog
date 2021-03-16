# ezlog

## 1. 简介

ezlog（easy log）是一款使用C/C++编写的简单易用的高性能日志库，支持同步/异步写入，日志滚动等功能。

## 2. 目录结构

```
.                               // 项目根目录
├── include                     // 导出的头文件
├── src                         // 内部实现
├── test                        // 测试例程
├── build-linux.sh              // 编译脚本，针对Linux平台
├── build-win32.bat             // 编译脚本，针对Win32平台
├── test_ezlog.sh               // 测试脚本，针对Linux平台
└── test_ezlog.bat              // 测试脚本，针对Win32平台
```

## 3. 开始使用

执行 **build-linux.sh** 或 **build-win32.bat** 编译库，编译完成后将会在项目根目录下 **built** 目录中得到 **win32/ezlog.lib** 或 **linux/libezlog.a** 文件

以下是简单的使用例子：

```c
// Step1: 在程序全局入口初始化日志库
ezlog_init();

// Step2: 设置日志等级
ezlog_set_level(EZLOG_LEVEL_VERBOSE);

// Step2.1（非必须）: 设置日志等级对应的输出格式，默认全部等级都为 EZLOG_FORMAT_ALL，即输出所有信息。
// ezlog_set_format(EZLOG_LEVEL_FATAL, EZLOG_FORMAT_ALL); // 对于 FATAL 等级的日志，输出所有信息
// ezlog_set_format(EZLOG_LEVEL_WARN, EZLOG_FORMAT_THREAD_INFO | EZLOG_FORMAT_FUNC_INFO); // 对于 WARN 等级日志，输出线程号与所在函数名

// Step3: 设置获取输出目录的钩子函数
ezlog_set_get_output_path_hook(get_output_path_hook);

// Step4: 现在你可以开始记录日志了：
LOG_FATAL("Hello World!");
LOG_ERROR("Hello World!");
LOG_WARN("Hello World!");
LOG_INFO("Hello World!");
LOG_DEBUG("Hello World!");
LOG_VERBOSE("Hello World!");
// 输出十六进制
unsigned char bytes[] = { 0x01, 0x02, 0x03 };
LOG_HEX(bytes, sizeof(bytes)); // 将会输出 `Hex of bytes: 010203`
```

## 4. 常量说明

### 4.1. 日志等级

| 名称                | 说明                                                         |
| ------------------- | ------------------------------------------------------------ |
| EZLOG_LEVEL_FATAL   | FATAL级别，DEBUG、VERBOSE、INFO、WARN、ERROR级别的日志将会被忽略 |
| EZLOG_LEVEL_ERROR   | ERROR级别，DEBUG、VERBOSE、INFO、WARN级别的日志将会被忽略    |
| EZLOG_LEVEL_WARN    | WARN级别，DEBUG、VERBOSE、INFO级别的日志将会被忽略           |
| EZLOG_LEVEL_INFO    | INFO级别，DEBUG、VERBOSE级别的日志将会被忽略                 |
| EZLOG_LEVEL_DEBUG   | DEBUG级别，VERBOSE级别的日志将会被忽略                       |
| EZLOG_LEVEL_VERBOSE | VERBOSE级别，所有等级日志都会输出                            |

### 4.2. 日志格式控制

以下常量可以通过位或操作进行组合

| 名称                     | 说明                         |
| ------------------------ | ---------------------------- |
| EZLOG_FORMAT_FUNC_INFO   | 控制是否输出函数信息         |
| EZLOG_FORMAT_FILE_INFO   | 控制是否输出源文件路径       |
| EZLOG_FORMAT_LINE_INFO   | 控制是否输出代码行号         |
| EZLOG_FORMAT_THREAD_INFO | 控制是否输出线程号           |
| EZLOG_FORMAT_ALL         | 启用以上所有EZLOG_FORMAT_*值 |

### 4.3. 其他常量

| 名称         | 说明                                                         |
| ------------ | ------------------------------------------------------------ |
| EZLOG_STDOUT | 标准输出，一般会输出到控制台，可以在 **ezlog_set_get_output_path_hook** 的钩子函数中使用 |