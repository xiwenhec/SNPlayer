//
// Created by Sivin on 2022-11-26.
//

#ifndef SN_FRAMEWORK_SNLOG_H
#define SN_FRAMEWORK_SNLOG_H

#include <mutex>
#include <memory>

#ifndef LOG_TAG
#define LOG_TAG ""
#endif



#define SN_LOGI(...) SNLog::info(LOG_TAG, __VA_ARGS__)
#define SN_LOGD(...) SNLog::debug(LOG_TAG, __VA_ARGS__)
#define SN_LOGW(...) SNLog::warn(LOG_TAG, __VA_ARGS__)
#define SN_LOGE(...) SNLog::error(LOG_TAG, __VA_ARGS__)


#define SN_TRACE do { SN_LOGD("%s:%d(%s)",__FILE__,__LINE__,__func__);} while(0)

class SNLog {
public:
    static void info(const char *tag, const char *fmt, ...);

    static void debug(const char *tag, const char *fmt, ...);

    static void warn(const char *tag, const char *fmt, ...);

    static void error(const char *tag, const char *fmt, ...);

private:
    SNLog();

    static std::shared_ptr<SNLog> getInstance();

    static void init();

    void snLogPrint(int priority, const char *tag, const char *fmt, va_list args);

    void formatLog(int priority, const char *tag, char *inBuffer, char *outBuffer);


private:

    static std::once_flag sLogInitFlag;
    static std::shared_ptr<SNLog> sInstance;

    std::string mVersion{"0.1"};
    int mPriority{0};
    std::mutex mLogMutex;
    bool mDisableConsole{false};
    bool mDisableLogColor{false};
    bool mShowPid{true};
};


#endif //SN_FRAMEWORK_SNLOG_H
