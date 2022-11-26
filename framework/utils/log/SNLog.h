//
// Created by Sivin on 2022-11-26.
//

#ifndef SIVINPLAYER_SNLOG_H
#define SIVINPLAYER_SNLOG_H

#define SNLOGI(...) SNLog::info(__VA_ARGS__)
#define SNLOGD(...) SNLog::debug(__VA_ARGS__)
#define SNLOGE(...) SNLog::error(__VA_ARGS__)

class SNLog {
public:
    static void info(...);
    static void debug(...);
    static void error(...);
};


#endif //SIVINPLAYER_SNLOG_H
