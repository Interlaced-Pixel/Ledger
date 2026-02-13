# Ledger

A thread-safe, feature-rich C++ logging library that provides flexible logging with multiple sinks, customizable formatters, and structured logging capabilities.

## Features

- **Thread-Safe Logging**: Full thread-safety with minimal lock contention
- **Multiple Log Levels**: TRACE, DEBUG, INFO, WARNING, ERROR, and FATAL severity levels
- **Flexible Log Sinks**:
  - Stream sinks for stdout/stderr and custom streams
  - File sinks with automatic rotation (size-based or time-based)
  - Asynchronous sinks for non-blocking logging with configurable drop policies
- **Custom Formatters**:
  - Default formatter with customizable timestamp formats
  - JSON formatter for structured log analysis
- **Structured Logging**: Key-value pairs in log messages for rich context
- **Log Context Storage**: Thread-local context that automatically includes context in all logs
- **Platform Support**: Cross-platform compatibility (Windows, macOS, Linux)
- **Compile-Time Filtering**: Optional compile-time log level filtering to reduce binary size
- **Category Loggers**: Per-module loggers with individual configurations

## Building

This is a header-only library. Include `ledger.h` in your project and compile with C++17 or later.

### C++ Standard

- Requires: **C++17** or later
- Platform-specific: Uses `localtime_s` on Windows, `localtime_r` on Unix-like systems

### Integration

1. Copy `ledger.h` to your project's include directory
2. Include the header in your source files:
   ```cpp
   #include "ledger.h"
   ```

## Quick Start

### Basic Logging

```cpp
using namespace pixellib::core::logging;

// Simple logging
Logger::info("Application started");
Logger::warning("This is a warning");
Logger::error("An error occurred");
```

### Setting Log Levels

```cpp
Logger::set_level(LOG_DEBUG);  // Enable all logs at DEBUG level and above
Logger::set_level(LOG_WARNING); // Only WARNING, ERROR, and FATAL
```

### File Logging with Rotation

```cpp
// Size-based rotation (10MB files, keep 5 files)
Logger::set_file_logging("app.log", 10485760, 5);

// Time-based rotation (daily rotation, keep 7 files)
Logger::set_file_logging("app.log", std::chrono::hours(24), 7);
```

### Structured Logging with Key-Value Pairs

```cpp
Logger::info("User action performed", 
            "user_id", 12345, 
            "action", "login",
            "ip_address", "192.168.1.1");
// Output: [2025-02-13 10:30:45] [INFO] User action performed | user_id=12345 action=login ip_address=192.168.1.1
```

### Thread-Local Context

```cpp
{
    LogContext ctx;
    ctx.add("request_id", "abc-123");
    ctx.add("user_id", 42);
    
    Logger::info("Processing request");  // Automatically includes context
    Logger::debug("Detailed processing");  // Context included here too
} // Context automatically cleared on scope exit
```

### Custom Formatters

```cpp
// Use JSON formatter
Logger::set_formatter(std::make_unique<JSONLogFormatter>());

// Use custom timestamp format
Logger::set_formatter(
    std::make_unique<DefaultLogFormatter>(
        TimestampFormat::ISO8601,
        "MyApp"
    )
);
```

### Async Logging

```cpp
auto async_sink = std::make_unique<AsyncLogSink>(
    std::make_unique<RotatingFileLogger>("app.log", 10485760, 5),
    1024,  // queue size
    AsyncLogSink::DropPolicy::DROP_NEWEST
);

Logger::add_sink(std::move(async_sink));

// Flush remaining messages
Logger::async_flush();

// Shutdown cleanly
Logger::async_shutdown();
```

### Fluent Configuration API

```cpp
Logger::configure(
    Logger::LoggerConfigBuilder()
        .set_level(LOG_DEBUG)
        .add_stream_sink(std::cout)
        .add_async_file_sink("app.log", 1024, AsyncLogSink::DropPolicy::DROP_NEWEST)
        .set_formatter(std::make_unique<JSONLogFormatter>())
        .build()
);
```

### Category Loggers

```cpp
auto db_logger = Logger::get("database");
db_logger.info("Connected to database");

auto api_logger = Logger::get("api");
api_logger.warning("High latency detected");

// Configure category-specific settings
Logger::LoggerRegistry::set_config(
    "database",
    Logger::LoggerConfigBuilder()
        .set_level(LOG_DEBUG)
        .add_file_sink("database.log")
        .build()
);
```

## Log Levels

| Level | Value | Use Case |
|-------|-------|----------|
| TRACE | 0 | Very detailed diagnostic information (compile-time disabled by default) |
| DEBUG | 1 | Detailed information for development and debugging |
| INFO | 2 | General operational information (default level) |
| WARNING | 3 | Warning messages about potential issues |
| ERROR | 4 | Error conditions |
| FATAL | 5 | Critical errors requiring immediate attention |

## Architecture

### Core Components

- **LogLevel**: Enumeration for severity levels with macro-based compile-time filtering
- **LogSink**: Abstract interface for pluggable log destinations
  - `StreamSink`: Writes to std::ostream
  - `RotatingFileLogger`: File output with automatic rotation
  - `AsyncLogSink`: Async wrapper around any sink
- **LogFormatter**: Abstract interface for message formatting
  - `DefaultLogFormatter`: Standard text format
  - `JSONLogFormatter`: Machine-parseable JSON format
- **Logger**: Main logging class with static methods
- **LogContext**: RAII helper for thread-local context management
- **CategoryLogger**: Named logger for per-module configuration

### Thread Safety

- All operations use mutex-protected critical sections
- Log formatting happens outside the critical section to minimize lock time
- Thread-local storage for log context avoids synchronization overhead

### Performance Considerations

- Early exit for messages below current log level (minimal overhead when disabled)
- Non-blocking async logging option for high-throughput scenarios
- Configurable queue size and drop policies for async sinks
- Header-only implementation allows compiler optimizations

## Compile-Time Configuration

### Log Level Filtering

Reduce binary size by filtering out low-severity logs at compile time:

```cpp
#define LEDGER_COMPILED_LOG_LEVEL LEDGER_LOG_LEVEL_INFO
#include "ledger.h"
```

This removes TRACE and DEBUG logging calls entirely from the compiled binary.

## Advanced Usage

### Custom Sinks

Implement the `LogSink` interface to create custom destinations:

```cpp
class CustomSink : public LogSink {
public:
    void write(const std::string& message) override {
        // Your custom logic here
    }
};

Logger::add_sink(std::make_unique<CustomSink>());
```

### Error Handling

The logger is designed to be robust:
- IO errors fall back to stderr
- Exceptions in sinks are caught and logged to stderr
- Stream state errors are recovered via `clear()`

## License

Copyright (c) 2025 Interlaced Pixel

Licensed under the **PolyForm Noncommercial License 1.0.0**. See [LICENSE](LICENSE) file for details.

**Summary**: This software is free for noncommercial use. Personal projects, research, education, and noncommercial organizations can use this software. Commercial use requires a license from Interlaced Pixel.

For more information, visit: https://polyformproject.org/licenses/noncommercial/1.0.0

## Contributing

Contributions are welcome! Please ensure:
- Code follows the existing style
- All changes are thread-safe
- New features include appropriate error handling
- Changes work across supported platforms

## Support

For issues, questions, or suggestions, please open an issue on the [GitHub repository](https://github.com/Interlaced-Pixel/Ledger).
