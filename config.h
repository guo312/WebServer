#ifndef CONFIG_H
#define CONFIG_H

#include "webserver.h"

using namespace std;

class Config
{
public:
    Config();
    ~Config(){};

    void parse_arg(int argc, char*argv[]);

    //服务端绑定端口号默认 9006 -p 可自定义
    int PORT;

    //日志写入方式默认同步写入 -l 可自定义 0 同步 1 异步
    int LOGWrite;

    //触发组合模式listenfd和connfd的模式组合，默认使用LT + LT0，-m 可自定义 0 表示使用LT + LT1 1 表示使用LT + ET2 2 表示使用ET + LT3 3 表示使用ET + ET
    int TRIGMode;

    //listenfd触发模式
    int LISTENTrigmode;

    //connfd触发模式
    int CONNTrigmode;

    //优雅关闭链接默认不使用 -o 可自定义 0 不使用 1 使用
    int OPT_LINGER;

    //数据库连接池数量默认为8 -s 可自定义
    int sql_num;

    //线程池内的线程数量默认为8 -t 可自定义
    int thread_num;

    //是否关闭日志默认不关闭 -c 可自定义 0 不关闭 1关闭
    int close_log;

    //并发模型选择默认Proactor -a 可自定义 0 Proactor模型 1 Reactor模型
    int actor_model;
};

#endif