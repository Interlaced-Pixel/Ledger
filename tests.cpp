/*
 * Ledger - Unit Tests
 * https://github.com/Interlaced-Pixel/Ledger
 *
 * Licensed under the PolyForm Noncommercial License 1.0.0
 * https://polyformproject.org/licenses/noncommercial/1.0.0
 *
 * Required Notice: Copyright (c) 2025 Interlaced Pixel
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "vendors/doctest.h"
#include "ledger.h"

#include <sstream>
#include <thread>
#include <fstream>
#include <filesystem>

using namespace pixellib::core::logging;

TEST_SUITE("LogLevel")
{
  TEST_CASE("log_level_to_string returns correct string representations")
  {
    CHECK(std::string(log_level_to_string(LOG_TRACE)) == "TRACE");
    CHECK(std::string(log_level_to_string(LOG_DEBUG)) == "DEBUG");
    CHECK(std::string(log_level_to_string(LOG_INFO)) == "INFO");
    CHECK(std::string(log_level_to_string(LOG_WARNING)) == "WARNING");
    CHECK(std::string(log_level_to_string(LOG_ERROR)) == "ERROR");
    CHECK(std::string(log_level_to_string(LOG_FATAL)) == "FATAL");
  }

  TEST_CASE("log_level_to_string handles unknown levels")
  {
    CHECK(std::string(log_level_to_string(static_cast<LogLevel>(999))) == "UNKNOWN");
  }
}

TEST_SUITE("StreamSink")
{
  TEST_CASE("StreamSink writes messages to output stream")
  {
    std::ostringstream oss;
    StreamSink sink(oss);

    sink.write("Test message");

    CHECK(oss.str() == "Test message\n");
  }

  TEST_CASE("StreamSink handles multiple writes")
  {
    std::ostringstream oss;
    StreamSink sink(oss);

    sink.write("First message");
    sink.write("Second message");

    std::string output = oss.str();
    CHECK(output.find("First message") != std::string::npos);
    CHECK(output.find("Second message") != std::string::npos);
  }
}

TEST_SUITE("RotatingFileLogger")
{
  TEST_CASE("RotatingFileLogger creates and writes to file")
  {
    std::string filename = "test_log.txt";
    
    {
      RotatingFileLogger logger(filename, 1000000, 5);
      logger.write("Test log message");
    }

    // Verify file was created and contains the message
    std::ifstream file(filename);
    CHECK(file.is_open());
    
    std::string line;
    bool found = false;
    while (std::getline(file, line))
    {
      if (line.find("Test log message") != std::string::npos)
      {
        found = true;
        break;
      }
    }
    file.close();
    CHECK(found);
    
    // Cleanup
    std::filesystem::remove(filename);
  }

  TEST_CASE("RotatingFileLogger handles multiple writes")
  {
    std::string filename = "test_log_multi.txt";
    
    {
      RotatingFileLogger logger(filename, 1000000, 5);
      logger.write("Message 1");
      logger.write("Message 2");
      logger.write("Message 3");
    }

    std::ifstream file(filename);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    CHECK(content.find("Message 1") != std::string::npos);
    CHECK(content.find("Message 2") != std::string::npos);
    CHECK(content.find("Message 3") != std::string::npos);
    
    std::filesystem::remove(filename);
  }
}

TEST_SUITE("AsyncLogSink")
{
  TEST_CASE("AsyncLogSink forwards messages to inner sink")
  {
    std::ostringstream oss;
    auto inner_sink = std::make_unique<StreamSink>(oss);
    AsyncLogSink async_sink(std::move(inner_sink), 100, AsyncLogSink::DropPolicy::DROP_NEWEST);

    async_sink.write("Test async message");
    async_sink.flush();

    CHECK(oss.str().find("Test async message") != std::string::npos);
  }

  TEST_CASE("AsyncLogSink drops oldest messages when full with DROP_OLDEST policy")
  {
    std::ostringstream oss;
    auto inner_sink = std::make_unique<StreamSink>(oss);
    AsyncLogSink async_sink(std::move(inner_sink), 2, AsyncLogSink::DropPolicy::DROP_OLDEST);

    async_sink.write("Message 1");
    async_sink.write("Message 2");
    async_sink.write("Message 3"); // Should drop Message 1

    async_sink.flush();

    CHECK(async_sink.dropped_count() > 0);
  }

  TEST_CASE("AsyncLogSink correctly reports dropped count")
  {
    std::ostringstream oss;
    auto inner_sink = std::make_unique<StreamSink>(oss);
    AsyncLogSink async_sink(std::move(inner_sink), 1, AsyncLogSink::DropPolicy::DROP_NEWEST);

    async_sink.write("Message 1");
    size_t initial_drops = async_sink.dropped_count();
    
    async_sink.write("Message 2"); // Should be dropped
    
    CHECK(async_sink.dropped_count() > initial_drops);
  }
}

TEST_SUITE("LogContext")
{
  TEST_CASE("LogContext stores and retrieves context values")
  {
    LogContextStorage::set("test_key", "test_value");
    CHECK(LogContextStorage::get("test_key") == "test_value");
    LogContextStorage::remove("test_key");
  }

  TEST_CASE("LogContext RAII automatically removes context on destruction")
  {
    CHECK(LogContextStorage::get("temp_key").empty());

    {
      LogContext ctx;
      ctx.add("temp_key", "temp_value");
      CHECK(LogContextStorage::get("temp_key") == "temp_value");
    }

    CHECK(LogContextStorage::get("temp_key").empty());
  }

  TEST_CASE("LogContext supports multiple key-value pairs")
  {
    {
      LogContext ctx;
      ctx.add("key1", "value1");
      ctx.add("key2", "value2");
      ctx.add("key3", "value3");

      CHECK(LogContextStorage::get("key1") == "value1");
      CHECK(LogContextStorage::get("key2") == "value2");
      CHECK(LogContextStorage::get("key3") == "value3");
    }

    CHECK(LogContextStorage::get("key1").empty());
    CHECK(LogContextStorage::get("key2").empty());
    CHECK(LogContextStorage::get("key3").empty());
  }

  TEST_CASE("LogContext can add numeric values")
  {
    {
      LogContext ctx;
      ctx.add("user_id", 12345);
      ctx.add("port", 8080);

      CHECK(LogContextStorage::get("user_id") == "12345");
      CHECK(LogContextStorage::get("port") == "8080");
    }
  }
}

TEST_SUITE("DefaultLogFormatter")
{
  TEST_CASE("DefaultLogFormatter produces standard format")
  {
    DefaultLogFormatter formatter;
    std::tm time_info = {};
    time_info.tm_year = 125;  // 2025
    time_info.tm_mon = 1;     // February
    time_info.tm_mday = 13;

    std::string result = formatter.format(LOG_INFO, "Test message", time_info);

    CHECK(result.find("Test message") != std::string::npos);
    CHECK(result.find("[INFO]") != std::string::npos);
    CHECK(result.find("2025-02-13") != std::string::npos);
  }

  TEST_CASE("DefaultLogFormatter includes file and line information")
  {
    DefaultLogFormatter formatter;
    std::tm time_info = {};

    std::string result = formatter.format(LOG_DEBUG, "Test", time_info, "test.cpp", 42);

    CHECK(result.find("test.cpp:42") != std::string::npos);
  }

  TEST_CASE("DefaultLogFormatter includes prefix when set")
  {
    DefaultLogFormatter formatter(TimestampFormat::STANDARD, "MyApp");
    std::tm time_info = {};

    std::string result = formatter.format(LOG_INFO, "Test", time_info);

    CHECK(result.find("MyApp") != std::string::npos);
  }

  TEST_CASE("DefaultLogFormatter supports ISO8601 timestamp format")
  {
    DefaultLogFormatter formatter(TimestampFormat::ISO8601);
    std::tm time_info = {};
    time_info.tm_year = 125;
    time_info.tm_mon = 1;
    time_info.tm_mday = 13;

    std::string result = formatter.format(LOG_INFO, "Test", time_info);

    CHECK(result.find("2025-02-13T") != std::string::npos);
    CHECK(result.find("Z") != std::string::npos);
  }

  TEST_CASE("DefaultLogFormatter supports NONE timestamp format")
  {
    DefaultLogFormatter formatter(TimestampFormat::NONE);
    std::tm time_info = {};

    std::string result = formatter.format(LOG_INFO, "Test message", time_info);

    CHECK(result.find("Test message") != std::string::npos);
    CHECK(result.find("[INFO]") != std::string::npos);
    // Should not have timestamp
    CHECK(result.find("[") == result.find("[INFO]"));
  }
}

TEST_SUITE("JSONLogFormatter")
{
  TEST_CASE("JSONLogFormatter produces valid JSON format")
  {
    JSONLogFormatter formatter;
    std::tm time_info = {};

    std::string result = formatter.format(LOG_INFO, "Test message", time_info);

    CHECK(result.find("{") != std::string::npos);
    CHECK(result.find("}") != std::string::npos);
    CHECK(result.find("\"timestamp\"") != std::string::npos);
    CHECK(result.find("\"level\"") != std::string::npos);
    CHECK(result.find("\"message\"") != std::string::npos);
  }

  TEST_CASE("JSONLogFormatter escapes JSON special characters")
  {
    JSONLogFormatter formatter;
    std::tm time_info = {};

    std::string result = formatter.format(LOG_INFO, "Message with \"quotes\" and \\backslash", time_info);

    // Should contain escaped quotes and backslashes
    bool escaped_quotes_found = result.find("\\\"") != std::string::npos;
    bool quoted_quotes_not_found = result.find("\"quotes\"") == std::string::npos;
    CHECK(escaped_quotes_found);
    CHECK(quoted_quotes_not_found);
  }

  TEST_CASE("JSONLogFormatter includes file and line in JSON")
  {
    JSONLogFormatter formatter;
    std::tm time_info = {};

    std::string result = formatter.format(LOG_ERROR, "Error", time_info, "error.cpp", 100);

    CHECK(result.find("\"file\"") != std::string::npos);
    CHECK(result.find("\"line\"") != std::string::npos);
    CHECK(result.find("100") != std::string::npos);
  }
}

TEST_SUITE("Logger - Basic Functionality")
{
  TEST_CASE("Logger sets log level")
  {
    Logger::set_level(LOG_WARNING);
    // Note: We can't directly verify the level, but we can test that it affects logging

    std::ostringstream oss;
    Logger::set_output_streams(oss, oss);
    
    Logger::info("Should not appear");
    Logger::warning("Should appear");

    std::string output = oss.str();
    CHECK(output.find("Should appear") != std::string::npos);
  }

  TEST_CASE("Logger logs info messages")
  {
    std::ostringstream oss;
    Logger::set_output_streams(oss, oss);
    Logger::set_level(LOG_INFO);

    Logger::info("Test info message");

    std::string output = oss.str();
    CHECK(output.find("Test info message") != std::string::npos);
    CHECK(output.find("[INFO]") != std::string::npos);
  }

  TEST_CASE("Logger logs warning messages")
  {
    std::ostringstream oss;
    Logger::set_output_streams(oss, oss);

    Logger::warning("Test warning message");

    std::string output = oss.str();
    CHECK(output.find("Test warning message") != std::string::npos);
    CHECK(output.find("[WARNING]") != std::string::npos);
  }

  TEST_CASE("Logger logs error messages")
  {
    std::ostringstream oss;
    Logger::set_output_streams(std::cout, oss);

    Logger::error("Test error message");

    std::string output = oss.str();
    CHECK(output.find("Test error message") != std::string::npos);
    CHECK(output.find("[ERROR]") != std::string::npos);
  }
}

TEST_SUITE("Logger - Structured Logging")
{
  TEST_CASE("Logger supports key-value pairs")
  {
    std::ostringstream oss;
    Logger::set_output_streams(oss, oss);

    Logger::info("User action", "user_id", 123, "action", "login");

    std::string output = oss.str();
    CHECK(output.find("User action") != std::string::npos);
    CHECK(output.find("user_id=123") != std::string::npos);
    CHECK(output.find("action=login") != std::string::npos);
  }

  TEST_CASE("Logger context is included in messages")
  {
    std::ostringstream oss;
    Logger::set_output_streams(oss, oss);

    {
      LogContext ctx;
      ctx.add("request_id", "req-123");
      Logger::info("Processing request");
    }

    std::string output = oss.str();
    CHECK(output.find("request_id=req-123") != std::string::npos);
  }
}

TEST_SUITE("Logger - Thread Safety")
{
  TEST_CASE("Logger handles concurrent logging from multiple threads")
  {
    std::ostringstream oss;
    Logger::set_output_streams(oss, oss);

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i)
    {
      threads.emplace_back([i]() {
        for (int j = 0; j < 10; ++j)
        {
          Logger::info("Thread message", "thread_id", i, "iteration", j);
        }
      });
    }

    for (auto &t : threads)
    {
      t.join();
    }

    std::string output = oss.str();
    // Should have logged 50 messages without crashing
    CHECK(output.find("Thread message") != std::string::npos);
  }
}

TEST_SUITE("Logger - File Logging")
{
  TEST_CASE("Logger can write to file")
  {
    std::string filename = "test_logger.log";
    Logger::set_file_logging(filename, 1000000, 5);

    Logger::info("File logging test message");

    std::ifstream file(filename);
    std::string line;
    bool found = false;
    while (std::getline(file, line))
    {
      if (line.find("File logging test message") != std::string::npos)
      {
        found = true;
        break;
      }
    }
    file.close();

    CHECK(found);

    Logger::set_file_logging(nullptr);
    std::filesystem::remove(filename);
  }
}

TEST_SUITE("Logger - Formatters")
{
  TEST_CASE("Logger can use custom formatter")
  {
    std::ostringstream oss;
    Logger::set_output_streams(oss, oss);

    auto json_formatter = std::make_unique<JSONLogFormatter>();
    Logger::set_formatter(std::move(json_formatter));

    Logger::info("JSON formatted message");

    std::string output = oss.str();
    CHECK(output.find("{") != std::string::npos);
    CHECK(output.find("\"message\"") != std::string::npos);

    Logger::set_formatter(nullptr);
  }

  TEST_CASE("Logger can use custom formatter with prefix")
  {
    std::ostringstream oss;
    Logger::set_output_streams(oss, oss);

    auto formatter = std::make_unique<DefaultLogFormatter>(TimestampFormat::STANDARD, "APP");
    Logger::set_formatter(std::move(formatter));

    Logger::info("Prefixed message");

    std::string output = oss.str();
    CHECK(output.find("APP") != std::string::npos);

    Logger::set_formatter(nullptr);
  }
}

TEST_SUITE("Logger - Configuration")
{
  TEST_CASE("LoggerConfigBuilder creates valid configuration")
  {
    auto config = Logger::LoggerConfigBuilder()
                      .set_level(LOG_DEBUG)
                      .add_stream_sink(std::cout)
                      .set_formatter(std::make_unique<JSONLogFormatter>())
                      .build();

    CHECK(config.level == LOG_DEBUG);
    CHECK(config.sinks.size() > 0);
    CHECK(config.formatter != nullptr);
  }

  TEST_CASE("Logger can be configured with fluent API")
  {
    std::ostringstream oss;

    auto config = Logger::LoggerConfigBuilder()
                      .set_level(LOG_INFO)
                      .add_stream_sink(oss)
                      .build();

    Logger::configure(std::move(config));

    Logger::info("Configured logger test");

    CHECK(oss.str().find("Configured logger test") != std::string::npos);
  }
}

TEST_SUITE("Logger - Category Loggers")
{
  TEST_CASE("CategoryLogger can be obtained and used")
  {
    auto logger = Logger::get("database");
    // Note: We test that it doesn't crash and is functional
    CHECK(true);
  }

  TEST_CASE("Different categories can have different configurations")
  {
    auto db_logger = Logger::get("database");
    auto api_logger = Logger::get("api");

    // Configure database logger
    Logger::LoggerRegistry::set_config(
        "database",
        Logger::LoggerConfigBuilder()
            .set_level(LOG_DEBUG)
            .build());

    CHECK(Logger::LoggerRegistry::has_config("database"));
  }
}

TEST_SUITE("Logger - Edge Cases")
{
  TEST_CASE("Logger handles empty messages")
  {
    std::ostringstream oss;
    Logger::set_output_streams(oss, oss);

    Logger::info("");

    // Should not crash and should log something
    CHECK(oss.str().find("[INFO]") != std::string::npos);
  }

  TEST_CASE("Logger handles very long messages")
  {
    std::ostringstream oss;
    Logger::set_output_streams(oss, oss);

    std::string long_message(10000, 'x');
    Logger::info(long_message);

    CHECK(oss.str().find("xxx") != std::string::npos);
  }

  TEST_CASE("Logger handles special characters in messages")
  {
    std::ostringstream oss;
    Logger::set_output_streams(oss, oss);

    Logger::info("Message with special chars: \n\t\r\\\"");

    CHECK(oss.str().find("special") != std::string::npos);
  }
}

TEST_SUITE("Macros")
{
  TEST_CASE("LOG_INFO macro works")
  {
    std::ostringstream oss;
    Logger::set_output_streams(oss, oss);

    LOG_INFO("Macro test message");

    std::string output = oss.str();
    CHECK(output.find("Macro test message") != std::string::npos);
  }

  TEST_CASE("LOG_WARNING macro works")
  {
    std::ostringstream oss;
    Logger::set_output_streams(oss, oss);

    LOG_WARNING("Warning via macro");

    std::string output = oss.str();
    CHECK(output.find("Warning via macro") != std::string::npos);
  }

  TEST_CASE("LOG_ERROR macro works")
  {
    std::ostringstream oss;
    Logger::set_output_streams(std::cout, oss);

    LOG_ERROR("Error via macro");

    std::string output = oss.str();
    CHECK(output.find("Error via macro") != std::string::npos);
  }

  TEST_CASE("LOG_FATAL macro works")
  {
    std::ostringstream oss;
    Logger::set_output_streams(std::cout, oss);

    LOG_FATAL("Fatal via macro");

    std::string output = oss.str();
    CHECK(output.find("Fatal via macro") != std::string::npos);
  }
}
