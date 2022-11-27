//
// Created by Sivin on 2022-11-26.
//

#include <thread>


#ifdef WIN32
#include <windows.h>
#else

#include <unistd.h>
#include <sys/time.h>

#endif

#include "SNLog.h"

#define APP_TAG "SNPlayer"

#define SNLOG_PRINTF_BUFFER_SIZE 1024

#define SN_LOG_LEVEL_NONE  0
#define SN_LOG_LEVEL_FATAL  8
#define SN_LOG_LEVEL_ERROR  16
#define SN_LOG_LEVEL_WARNING  24
#define SN_LOG_LEVEL_INFO  32
#define SN_LOG_LEVEL_DEBUG  48
#define SN_LOG_LEVEL_TRACE  56


#define NONE         "\033[m"
#define RED          "\033[0;32;31m"
#define LIGHT_RED    "\033[1;31m"
#define GREEN        "\033[0;32;32m"
#define LIGHT_GREEN  "\033[1;32m"
#define BLUE         "\033[0;32;34m"
#define LIGHT_BLUE   "\033[1;34m"
#define DARY_GRAY    "\033[1;30m"
#define CYAN         "\033[0;36m"
#define LIGHT_CYAN   "\033[1;36m"
#define PURPLE       "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN        "\033[0;33m"
#define YELLOW       "\033[1;33m"
#define LIGHT_GRAY   "\033[0;37m"
#define WHITE        "\033[1;37m"


static char getLevelChar(int lev, const char **ctrl) {
    *ctrl = NONE;
    switch (lev) {
        case SN_LOG_LEVEL_INFO:
            *ctrl = GREEN;
            return 'I';

        case SN_LOG_LEVEL_DEBUG:
            *ctrl = nullptr;
            return 'D';

        case SN_LOG_LEVEL_WARNING:
            *ctrl = YELLOW;
            return 'W';

        case SN_LOG_LEVEL_ERROR:
            *ctrl = RED;
            return 'E';

        case SN_LOG_LEVEL_FATAL:
            *ctrl = LIGHT_RED;
            return 'F';
        default:
            return ' ';
    }
}


std::once_flag SNLog::sLogInitFlag;
std::shared_ptr<SNLog> SNLog::sInstance{nullptr};

std::shared_ptr<SNLog> SNLog::getInstance() {
    std::call_once(sLogInitFlag, &SNLog::init);
    return sInstance;
}

void SNLog::init() {
    sInstance.reset(new SNLog());
}

SNLog::SNLog() {
    mVersion = "0.1";
#ifdef NDEBUG
    mPriority = SN_LOG_LEVEL_INFO;
#else
    mPriority = SN_LOG_LEVEL_DEBUG;
#endif
}


int SNLog::snLogPrint(int prio, const char *tag, const char *fmt, va_list args) {
    if (prio > mPriority) {
        return 0;
    }

    std::lock_guard<std::mutex> lockGuard{mLogMutex};
    static char printfBuf[SNLOG_PRINTF_BUFFER_SIZE] = {0};
    vsnprintf(printfBuf, SNLOG_PRINTF_BUFFER_SIZE - 1, fmt, args);

    if (!mDisableConsole) {

#if ANDROID || TARGET_OS_IPHONE
        static char finalLogMsg[SNLOG_PRINTF_BUFFER_SIZE + 512] = {0};
        sprintf(finalLogMsg, "[%s] [%s] :%s", mVersion.c_str(), tag, printfBuf);
#endif

#ifdef ANDROID
        __android_log_print(prio, APP_TAG, "%s", finalLogMsg);
#elif defined(__APPLE__) && TARGET_OS_IPHONE
        const char *ctr = NULL;
        appleNSlogC(get_char_lev(prio, &ctr), finalLogMsg);
#else
        static char outBuffer[SNLOG_PRINTF_BUFFER_SIZE + 512] = {0};
        formatLog(prio, tag, printfBuf, outBuffer);
        const char *ctrlStr = nullptr;
        getLevelChar(prio, &ctrlStr);
        if (!mDisableLogColor && ctrlStr) {
            printf("%s", ctrlStr);
        }

#ifdef WIN32
        OutputDebugString(printf_buf);
#endif

        printf("%s", outBuffer);

        if (!mDisableLogColor && ctrlStr) {
            printf("%s", NONE);
        }
#endif

        return 0;
    }
}


static void getLocalTime(char *buffer) {
#ifdef WIN32
    SYSTEMTIME st = { 0 };
    GetLocalTime(&st);
    sprintf(buffer, "%d-%02d-%02d %02d:%02d:%02d.%03d",
            st.wYear,
            st.wMonth,
            st.wDay,
            st.wHour,
            st.wMinute,
            st.wSecond,
            st.wMilliseconds);
#else
    struct timeval t{};
    gettimeofday(&t, nullptr);
    struct tm *ptm = localtime(&t.tv_sec);
    sprintf(buffer, "%02d-%02d %02d:%02d:%02d.%04d",
            ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec,
            (int) (t.tv_usec / 1000));
#endif
}


void SNLog::formatLog(int priority, const char *tag, char *inBuffer, char *outBuffer) {
    std::thread::id tid = std::this_thread::get_id();
    int pid = 0;
#ifndef WIN32
    pid = getpid();
#endif
    char timeBuffer[32];
    getLocalTime(timeBuffer);

    const char *ctr = nullptr;
    char levelChar = getLevelChar(priority, &ctr);

    if (mShowPid) {
        sprintf(outBuffer, "%s %d 0x%llx %c/%s [%s] [%s]: %s", timeBuffer, pid, tid, levelChar, APP_TAG,
                mVersion.c_str(), tag, inBuffer);
    } else {
        sprintf(outBuffer, "%s 0x%llx %c/%s [%s] [%s]: %s", timeBuffer, tid, levelChar, APP_TAG,
                mVersion.c_str(), tag, inBuffer);
    }
}


void SNLog::info(const char *tag, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    SNLog::getInstance()->snLogPrint(SN_LOG_LEVEL_INFO, tag, fmt, args);
    va_end(args);
}

void SNLog::debug(const char *tag, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    SNLog::getInstance()->snLogPrint(SN_LOG_LEVEL_DEBUG, tag, fmt, args);
    va_end(args);
}

void SNLog::error(const char *tag, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    SNLog::getInstance()->snLogPrint(SN_LOG_LEVEL_ERROR, tag, fmt, args);
    va_end(args);
}








