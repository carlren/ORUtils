// This file is a modified version of the logging file created by Ynagbin Lin.
//
// Copyright 2016 Yangbin Lin. All Rights Reserved.
//
// Author: yblin@jmu.edu.cn (Yangbin Lin)
//
// This file is part of the Code Library.
//
// Description: Logging framework in one file.
//
// Example Usage:
//
//    SCLOG_ON(INFO);
//    SCLOG(INFO) << "This is test info log";
//
#pragma once
#ifndef SCLOGGING_H_
#define SCLOGGING_H_

#include <chrono>
#include <cinttypes>
#include <ctime>
#include <memory>
#include <mutex>
#include <fstream>
#include <sstream>

namespace ORUtils {
    namespace logging {
        class Message {
        public:
            Message() : stream_(new std::stringstream) {}

            template<typename T>
            Message &operator<<(const T &value) {
                *stream_ << value;
                return *this;
            }

            const std::string str() const { return stream_->str(); }

            static std::string GetFileFunctionLine(const char *f, const char *func, int l){
                std::string path(f);
                std::string base_filename = path.substr(path.find_last_of("/\\") + 1);
                std::stringstream s;
                s << "["<<base_filename << ":"<<func<<":"<<l<<"]";
                return s.str();
            }
            static std::string GetFileFunction(const char *f, const char *func){
                std::stringstream s;
                s << "["<<f << ":"<<func<<"]";
                return s.str();
            }
            static std::string GetFunctionLine(const char *func, int l){
                std::stringstream s;
                s << "["<<func << ":"<<l<<"]";
                return s.str();
            }
        private:
            const std::unique_ptr<std::stringstream> stream_;
        };

/// The severity level for logger.

        enum Severity {
            INFO = 0,
            WARNING = 1,
            ERROR = 2,
            DEBUG = 3,
            VERBOSE = 4
        };

        struct Log {
            std::mutex m_mutex;
            std::vector<char> buffer;
            std::vector<int> LineOffsets;
            Log(){
                Clear();
            }

            void AddLog(const std::string &log) {
                std::lock_guard<std::mutex> lock(m_mutex);
                int old_size = buffer.size();
                buffer.insert(buffer.end(), log.begin(),log.end());
                for (int new_size = buffer.size(); old_size < new_size; old_size++)
                    if (buffer[old_size-1] == '\n')
                        LineOffsets.push_back(old_size);
            }

            void AddLog(const std::vector<char> &log) {
                std::lock_guard<std::mutex> lock(m_mutex);
                int old_size = buffer.size();
                buffer.insert(buffer.end(), log.begin(),log.end());
                for (int new_size = buffer.size(); old_size < new_size; old_size++)
                    if (buffer[old_size-1] == '\n')
                        LineOffsets.push_back(old_size);
            }

            void    Clear()
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                buffer.clear();
                LineOffsets.clear();
                LineOffsets.push_back(0);
            }

            void Print() {
                std::lock_guard<std::mutex> lock(m_mutex);
                std::string s(buffer.begin(),buffer.end());
                printf("%s\n",s.c_str());
            }

            void Append(std::vector<char> &out, size_t start){
                std::lock_guard<std::mutex> lock(m_mutex);
                out.insert(out.end(),buffer.begin()+start, buffer.end());
            }

            size_t size() {
                std::lock_guard<std::mutex> lock(m_mutex);
                return buffer.size();
            }

//            size_t GetStartEnd(){
//
//            }
        };

/**
 * A Logger is a center object of the whole logging system.
 *
 * This is a singleton class. The only instance of Logger is created when
 * Logger::GetInstance() is first called. This instance is never deleted.
 *
 * Logger is not copyable.
 */
        class Logger {
        private:
            Logger()
                    : severity_level_(INFO), mbLogToFile(false), mbLogToLogBuffer(false), mbLogToCout(true) {
                char buffer[1024];
                std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::tm *tm = localtime(&tt);
                sprintf(buffer, "log_%04d-%02d-%02d_%02d-%02d-%02d.txt",
                        tm->tm_year+1900, tm->tm_mon + 1, tm->tm_mday,
                        tm->tm_hour, tm->tm_min, tm->tm_sec);
                msLogName = buffer;
            }

        public:

            /// Store all log data.
            /**
             * A record has the severity of logging, the position where the file and line
             * call the log.
             */
            struct Record {
                Record(const Severity &s, const char *f, const char *func, int l)
                        : severity(s), filename(f), functionname(func), line(l) {
                    size_t t1 = filename.find_last_of('\\');
                    size_t t2 = filename.find_last_of('/');
                    if (t1 == std::string::npos) t1 = 0;
                    if (t2 == std::string::npos) t2 = 0;
                    if (t1 != 0 || t2 != 0)
                        filename = filename.substr(std::max(t1, t2) + 1);

                    time = std::chrono::system_clock::now();
                }

                /**
                 * A semantic trick to enable message streaming.
                 */
                Record &operator+=(const Message &m) {
                    message << m.str();

                    return *this;
                }

                // Severity level for logging.
                const Severity severity;

                // Source code filename.
                std::string filename;
                std::string functionname;

                // Source code line.
                const int line;

                // Message of record.
                Message message;
//        Message message;

                // Log time.
                std::chrono::time_point<std::chrono::system_clock> time;

            private:
                Record(const Record &) = delete;

                void operator=(const Record &) = delete;
            };

            /**
             * Get the singleton Logger object.
             * The first time this method is called, a Logger object is constructed and
             * returned.
             * Consecutive calls will return the same object.
             */
            static Logger *GetInstance() {
                static Logger instance;
                return &instance;
            }

            /**
             * A semantic trick to enable logging streaming.
             */
            void operator+=(const Record &record) {
                std::lock_guard<std::mutex> guard(mutex_);
                ss.str("");
                ss.clear();
                // Print severity.
                ss << SeverityToString(record.severity);

                // Print date time.
                std::time_t tt = std::chrono::system_clock::to_time_t(record.time);
                std::tm *tm = localtime(&tt);
                char buffer[1024];
                sprintf(buffer, "%02d%02d %02d:%02d:%02d",
                        tm->tm_mon + 1, tm->tm_mday,
                        tm->tm_hour, tm->tm_min, tm->tm_sec);
                ss << buffer;

                // Print milliseconds.
                auto since_epoch = record.time.time_since_epoch();
                std::chrono::seconds s =
                        std::chrono::duration_cast<std::chrono::seconds>(since_epoch);
                since_epoch -= s;

                typedef std::chrono::milliseconds Milliseconds;
                Milliseconds milliseconds =
                        std::chrono::duration_cast<Milliseconds>(since_epoch);
                sprintf(buffer, ".%03" PRId64 "", milliseconds.count());
                ss << buffer;

                // Print message.
                sprintf(buffer, " %s:%s:%d] ", record.filename.c_str(),record.functionname.c_str(), record.line);
                ss << buffer;
                sprintf(buffer, "%s\n", record.message.str().c_str());
                ss << buffer;

                if (CheckSeverity(record.severity)) {
                    if(mbLogToFile) {
                        if(f == nullptr) f.reset(new std::fstream(msLogName, std::ios::out));
                        else f->open(msLogName, std::ios::out | std::ios::app);
                        (*f) << ss.str();
                        f->close();
                    }
                    if(mbLogToCout)
                        printf("%s", ss.str().c_str());
                    if(mbLogToLogBuffer)
                        mLogBuffer.AddLog(ss.str());
                }
                if (record.severity == Severity::ERROR) throw std::runtime_error(ss.str());
                fflush(stdout);
            }

            /**
             * Check if severity is valid.
             */
            bool CheckSeverity(const Severity &severity) const {
                return severity <= severity_level_;
            }

            void set_severity_level(const Severity &severity_level) {
                std::lock_guard<std::mutex> guard(mutex_);
                severity_level_ = severity_level;
            }

            void set_log_to_file(bool option){
                std::lock_guard<std::mutex> guard(mutex_);
                mbLogToFile = option;
            }

            void set_log_to_cout(bool option) {
                std::lock_guard<std::mutex> guard(mutex_);
                mbLogToCout = option;
            }

            void set_log_to_buffer(bool option) {
                std::lock_guard<std::mutex> guard(mutex_);
                mbLogToLogBuffer = option;
            }

            const Severity &severity_level() const {
                return severity_level_;
            }

            Log *GetLogBuffer(){return &mLogBuffer;}
        private:
            /**
             * Convert serverity to string.
             */
            static const char *SeverityToString(Severity severity) {
                switch (severity) {
                    case INFO:
                        return "I";
                    case WARNING:
                        return "W";
                    case ERROR:
                        return "E";
                    case DEBUG:
                        return "D";
                    case VERBOSE:
                        return "V";
                    default:
                        return "N";
                }
            }

            // The logger severity upper limit.
            // All log messages have its own severity and if it is higher than the limit
            // those messages are dropped.
            Severity severity_level_;

            std::stringstream ss;

            // Mutex for thread safe.
            std::mutex mutex_;

            bool mbLogToFile;

            bool mbLogToLogBuffer;

            bool mbLogToCout;

            std::unique_ptr<std::fstream> f;

            std::string msLogName;

            Log mLogBuffer;

            Logger(const Logger &);

            void operator=(const Logger &);
        };

    } // namespace tool
} // namespace log

/**
 * Enable the namespace for log severity.
 */
#define SCLOG_SEVERITY(severity) ORUtils::logging::severity

/**
 * Main logging macros.
 *
 * Example usage:
 *
 *   SCLOG(INFO) << "This is a info log";
 */
#define SCLOG(severity) \
        (*ORUtils::logging::Logger::GetInstance()) += \
            ORUtils::logging::Logger::Record(SCLOG_SEVERITY(severity), __FILE__, __FUNCTION__, __LINE__) += \
            ORUtils::logging::Message()

#define GETFFL ORUtils::logging::Message::GetFileFunctionLine( __FILE__, __FUNCTION__, __LINE__)
#define GETFF ORUtils::logging::Message::GetFileFunction( __FILE__, __FUNCTION__)
#define GETFL ORUtils::logging::Message::GetFunctionLine( __FUNCTION__, __LINE__)
/**
 * Turn on the log and set the severity level.
 *
 * Example usage:
 *
 *    SCLOG_ON(INFO);
 */
#define SCLOG_ON(severity) \
    ORUtils::logging::Logger::GetInstance()->set_severity_level(SCLOG_SEVERITY(severity))
#define SCLOG_TO_FILE(option) \
    ORUtils::logging::Logger::GetInstance()->set_log_to_file(option)
#define SCLOG_TO_COUT(option) \
    ORUtils::logging::Logger::GetInstance()->set_log_to_cout(option)
#define SCLOG_TO_BUFFER(option) \
    ORUtils::logging::Logger::GetInstance()->set_log_to_buffer(option)

#define SCLOGBUFFER ORUtils::logging::Logger::GetInstance()->GetLogBuffer()
#endif // SCLOGGING_H_
