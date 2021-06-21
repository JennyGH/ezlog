# ezlog

## 1. 简介

一个随便写的C/C++日志库，支持同步/异步输出日志，支持滚动日志

## 2. 目录结构

```
.                               // 项目根目录
├── include                     // 导出的头文件
├── src                         // 内部实现
├── test                        // 测试例程
├── example                     // 使用示例
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

最终会在 `./built/win32` 中得到编译后的库文件与头文件，其中 `.dll` 文件存放在 `bin` 目录

### 3.2. Linux 平台

```bash
$ cd ezlog
$ bash build-linux.sh
```

最终会在 `./built/linux` 中得到编译后的库文件与头文件

## 3. 开始使用

来看看怎么用：

```c
const char* _get_output_path_hook() {
    return "/var/log/ezlog_output.log";
}
bool _roll_hook(unsigned long file_size) {
    // 当前日志大于等于10MB时创建新的日志文件
    return file_size >= 1024 * 1024 * 10;
}

// 使用 ezlog_write_log_args 封装为自己的日志函数
static void _my_log_function(
    unsigned int level,
    const char*  function,
    const char*  file,
    unsigned int line,
    const char*  format,
    ...)
{
    va_list args;
    va_start(args, format);
    ezlog_write_log_args(level, function, file, line, format, args);
    va_end(args);
}

// 自定义断言回调
static void _assert_hook(const char* expr, const char* file, unsigned int line)
{
    _my_log_function(
        EZLOG_LEVEL_FATAL,
        __FUNCTION__,
        file,
        line,
        "Assert fail: `%s` is `false`!",
        expr);
    // while (true) {} // 原地自旋
}

int main(int argc, char* argv[])
{
    static unsigned char bytes[] = {0x00, 0x01, 0x02, 0x03, 0x04};

    // 初始化日志库
    ezlog_init();

    // 设置日志等级
    ezlog_set_level(EZLOG_LEVEL_VERBOSE);

    // （可选）我想让日志变得好看一点
    ezlog_set_log_color_enabled(true);

    // （可选）同步输出太慢了，用异步可能会快一点
    // ezlog_set_async_mode_enabled(true);

    // （可选）告诉我要多大的缓冲区去存放异步日志内容吧
    // ezlog_set_async_buffer_size(1024 * 1024);

    // （可选）我想某些等级的日志不要输出太多没用信息
    ezlog_set_format(EZLOG_LEVEL_FATAL,   EZLOG_FORMAT_ALL); // 输出全部信息
    ezlog_set_format(EZLOG_LEVEL_ERROR,   EZLOG_FORMAT_ALL);
    ezlog_set_format(EZLOG_LEVEL_WARN,    EZLOG_FORMAT_FUNC_INFO); // 只输出所在函数信息
    ezlog_set_format(EZLOG_LEVEL_INFO,    EZLOG_FORMAT_FUNC_INFO);
    ezlog_set_format(EZLOG_LEVEL_DEBUG,   EZLOG_FORMAT_ALL & (~EZLOG_FORMAT_FUNC_INFO)); // 不输出所在函数信息
    ezlog_set_format(EZLOG_LEVEL_VERBOSE, EZLOG_FORMAT_NONE); // 只输出日志时间与日志内容

    // （可选）告诉我 EZLOG_ASSERT 的时候应该做什么
    ezlog_set_assert_hook(_assert_hook);
    // （可选）启用日志滚动
    ezlog_set_log_roll_enabled(true);
    // （可选）告诉我该以什么方式来判断是否应该滚动日志了
    ezlog_set_roll_hook(_roll_hook);
    // 告诉我日志该输出到哪里
    ezlog_set_get_output_path_hook(_get_output_path_hook);

    // 输出些东西
    LOG_FATAL("Test verbose log.");
    LOG_ERROR("Test error log.");
    LOG_WARN("Test warn log.");
    LOG_INFO("Test info log.");
    LOG_DEBUG("Test debug log.");
    LOG_VERBOSE("Test verbose log.");

    // 简单地输出十六进制
    LOG_HEX(bytes, sizeof(bytes));

    // 以VERBOSE等级带自定义前缀地输出十六进制
    ezlog_write_hex(
        EZLOG_LEVEL_VERBOSE,
        "Output hex of `bytes` with custom prefix: ",
        bytes,
        sizeof(bytes));

    // 断言
    EZLOG_ASSERT(sizeof(bytes) >= 1024);

    // 释放日志库资源
    ezlog_deinit();
    return 0;
}
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
