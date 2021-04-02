# ezlog

## 1. 简介

ezlog（easy log）是一款使用 C/C++编写的简单易用的高性能日志库，支持同步/异步写入，日志滚动等功能。

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

## 3. 编译

### 3.1. Windows 平台

运行开发者命令提示符（如 Visual Studio 2005 命令提示），执行以下命令：

```bash
> cd /d ezlog
> build-win32.bat
```

最终会在 ezlog/built/win32/lib 中得到编译后的 ezlog.lib 文件，如果编译动态库，会在 ezlog/built/win32/bin 中得到 ezlog.dll 文件

### 3.2. Linux 平台

```bash
$ cd ezlog
$ bash build-linux.sh
```

最终会在 ezlog/built/linux/lib 中得到编译后的 libezlog.a 文件

## 3. 开始使用

以下是简单的使用例子：

```c
const char* _get_output_path() {
    return "/var/log/ezlog_output.log";
}
bool _should_roll_log(unsigned long file_size) {
    // 当前日志大于等于10MB时创建新的日志文件
    return file_size >= 1024*1024*10;
}
void _assert_triggered(const char* expr, const char* file, unsigned int line) {
    // 记录日志
    LOG_FATAL("Oops! Assert failed at file: %s, line: %u", file, line);
    // 然后原地自旋
    while(true) {}
}

// Step1: 在程序全局入口初始化日志库
ezlog_init();

// Step2: 设置日志等级
ezlog_set_level(EZLOG_LEVEL_VERBOSE);

// Step2.1（非必须）: 设置日志等级对应的输出格式，默认全部等级都为 EZLOG_FORMAT_ALL，即输出所有信息。
// ezlog_set_format(EZLOG_LEVEL_FATAL, EZLOG_FORMAT_ALL); // 对于 FATAL 等级的日志，输出所有信息
// ezlog_set_format(EZLOG_LEVEL_WARN, EZLOG_FORMAT_THREAD_INFO | EZLOG_FORMAT_FUNC_INFO); // 对于 WARN 等级日志，输出线程号与所在函数名
// ezlog_set_log_roll_enabled(true);         // 是否开启日志滚动
// ezlog_set_roll_hook(_should_roll_log);    // 设置日志滚动的钩子函数
// ezlog_set_assert_hook(_assert_triggered); // 设置使用 EZLOG_ASSERT 宏时的钩子函数
// ezlog_set_log_color_enabled(true); // 是否启用颜色输出（仅Linux下有效）
// ezlog_set_async_mode_enabled(true); // 是否启用异步日志模式
// ezlog_set_async_buffer_size(1024*1024); // 设置异步日志缓冲区大小：1MB

// Step3: 设置获取输出目录的钩子函数
ezlog_set_get_output_path_hook(_get_output_path);

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
// 表达式为false，将会触发通过 ezlog_set_assert_hook 设置的断言钩子函数
EZLOG_ASSERT(sizeof(bytes) > 3);

// Step5: 释放资源，如果是异步模式，将会输出异步缓冲区中剩余的日志
ezlog_deinit();
```

## 4. 常量说明

### 4.1. 日志等级

| 名称                | 说明                                                               |
| ------------------- | ------------------------------------------------------------------ |
| EZLOG_LEVEL_FATAL   | FATAL 级别，DEBUG、VERBOSE、INFO、WARN、ERROR 级别的日志将会被忽略 |
| EZLOG_LEVEL_ERROR   | ERROR 级别，DEBUG、VERBOSE、INFO、WARN 级别的日志将会被忽略        |
| EZLOG_LEVEL_WARN    | WARN 级别，DEBUG、VERBOSE、INFO 级别的日志将会被忽略               |
| EZLOG_LEVEL_INFO    | INFO 级别，DEBUG、VERBOSE 级别的日志将会被忽略                     |
| EZLOG_LEVEL_DEBUG   | DEBUG 级别，VERBOSE 级别的日志将会被忽略                           |
| EZLOG_LEVEL_VERBOSE | VERBOSE 级别，所有等级日志都会输出                                 |

### 4.2. 日志格式控制

以下常量可以通过位或操作进行组合

| 名称                     | 说明                           |
| ------------------------ | ------------------------------ |
| EZLOG_FORMAT_FUNC_INFO   | 控制是否输出函数信息           |
| EZLOG_FORMAT_FILE_INFO   | 控制是否输出源文件路径         |
| EZLOG_FORMAT_LINE_INFO   | 控制是否输出代码行号           |
| EZLOG_FORMAT_THREAD_INFO | 控制是否输出线程号             |
| EZLOG_FORMAT_ALL         | 启用以上所有 EZLOG*FORMAT*\*值 |

### 4.3. 其他常量

| 名称         | 说明                                                                                     |
| ------------ | ---------------------------------------------------------------------------------------- |
| EZLOG_STDOUT | 标准输出，一般会输出到控制台，可以在 **ezlog_set_get_output_path_hook** 的钩子函数中使用 |
