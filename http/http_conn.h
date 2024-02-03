#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <map>

#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"
#include "../timer/lst_timer.h"
#include "../log/log.h"

class http_conn
{
public:
    static const int FILENAME_LEN = 200;
    // 读缓冲区大小
    static const int READ_BUFFER_SIZE = 2048;
    // 写缓冲区大小
    static const int WRITE_BUFFER_SIZE = 1024;
    // http 协议的请求类型 GET 登录是使用 POST 注册时使用
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    // 解析请求主状态机的几种状态 
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,  // 请求首行
        CHECK_STATE_HEADER,           // 头部
        CHECK_STATE_CONTENT           // 请求体
    };
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    // 解析请求从状态机
    enum LINE_STATUS
    {
        LINE_OK = 0,                  // 行解析完毕
        LINE_BAD,                     // 行消息出错
        LINE_OPEN                     // 行消息不完整
    };

public:
    http_conn() {}
    ~http_conn() {}

public:
    // 主要初始化和连接相关的参数
    void init(int sockfd, const sockaddr_in &addr, char *, int, int, string user, string passwd, string sqlname);
    // 关闭连接
    void close_conn(bool real_close = true);
    // 解析http请求生成响应报文
    void process();
    // 处理读事件
    bool read_once();
    bool write();
    sockaddr_in *get_address()
    {
        return &m_address;
    }
    void initmysql_result(connection_pool *connPool);
    int timer_flag;
    int improv;


private:
    // http 协议是有状态的协议 这里初始化和状态相关的参数
    void init();
    // 解析请求报文
    HTTP_CODE process_read();
    // 产生响应报文
    bool process_write(HTTP_CODE ret);
    //解析http请求行，获得请求方法，目标url及http版本号
    HTTP_CODE parse_request_line(char *text);
    //解析http请求的一个头部信息
    HTTP_CODE parse_headers(char *text);
    //判断http请求是否被完整读入
    HTTP_CODE parse_content(char *text);
    //根据解析内容做出响应
    HTTP_CODE do_request();
    char *get_line() { return m_read_buf + m_start_line; };
    //从状态机，用于分析出一行内容 返回值为行的读取状态，有LINE_OK,LINE_BAD,LINE_OPEN
    LINE_STATUS parse_line();
    void unmap();
    // 可变参数，实际生成响应报文
    bool add_response(const char *format, ...);
    // 添加响应报文消息体
    bool add_content(const char *content);
    // 添加响应首行
    bool add_status_line(int status, const char *title);
    // 添加响应报文头部
    bool add_headers(int content_length);
    // 添加响应报文类型字段 实际未调用
    bool add_content_type();
    // 添加响应报文消息体长度
    bool add_content_length(int content_length);
    // 添加是否长连接字段
    bool add_linger();
    // 添加空行
    bool add_blank_line();

public:
    static int m_epollfd;
    static int m_user_count;
    MYSQL *mysql;
    // 0 表示读状态 1 表示写状态 在 reactor模式下IO操作工作线程完成，工作线程需要知道当前状态，proactor 不需要
    int m_state; 

private:
    int m_sockfd;
    sockaddr_in m_address;
    char m_read_buf[READ_BUFFER_SIZE];  // 读缓存
    long m_read_idx;                    // 读缓存中报文长度
    long m_checked_idx;                 // 待解析请求报文下标
    int m_start_line;                   // 每行数据的起始下标
    char m_write_buf[WRITE_BUFFER_SIZE];
    int m_write_idx;
    CHECK_STATE m_check_state;
    METHOD m_method;
    char m_real_file[FILENAME_LEN];     // 请求服务端资源路径
    char *m_url;                        // 请求资源URL
    char *m_version;                    // http 协议版本号
    char *m_host;                       // 目标服务器的主机名或IP地址
    long m_content_length;              // 消息体长度
    bool m_linger;                      // 是否长连接
    char *m_file_address;
    struct stat m_file_stat;
    struct iovec m_iv[2];
    int m_iv_count;
    int cgi;        //是否是POST请求
    char *m_string; //存储请求头数据
    int bytes_to_send;
    int bytes_have_send;
    char *doc_root; //资源根目录

    map<string, string> m_users;
    int m_TRIGMode;
    int m_close_log;

    char sql_user[100];
    char sql_passwd[100];
    char sql_name[100];
};

#endif
