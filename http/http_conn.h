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
#include "../time/time_wheel_timer.h"
#include "../log/log.h"

class http_conn
{
public:
    static const int FILENAME_LEN = 200;    /* 文件名最大长度 */
    static const int READ_BUFFER_SIZE = 2048;   /* 读缓冲区的大小 */
    static const int WRITE_BUFFER_SIZE = 1024;  /* 写缓冲区的大小 */
    enum METHOD /* HTTP 请求方法 */
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATCH
    };
    enum CHECK_STATE    /* 解析客户端请求时，主状态机的状态 */
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum HTTP_CODE  /* 处理HTTP请求可能得到的结果 */
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
    enum LINE_STATUS    /* 行的读取状态 */
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

public:
    http_conn() {}
    ~http_conn() {}

public:
    /* 初始化新接受的连接 */
    void init(int sockfd, const sockaddr_in &addr, char *, int, int, string user, string passwd, string sqlname);
    void close_conn(bool real_close = true);    /* 关闭连接 */
    void process(); /* 处理客户请求 */
    bool read_once(); /*非阻塞读操作*/
    bool write();   /*非阻塞写操作*/
    sockaddr_in *get_address()
    {
        return &m_address;
    }
    void initmysql_result(connection_pool *connPool);
    int timer_flag;
    int improv;


private:
    void init();    /* 初始化连接 */
    HTTP_CODE process_read();   /* 解析HTTP请求 */
    bool process_write(HTTP_CODE ret); /* 填充HTTP应答 */
    // 被 process_read 调用分析 HTTP 请求
    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);
    HTTP_CODE do_request();
    char *get_line() { return m_read_buf + m_start_line; };
    LINE_STATUS parse_line();
    // 被 process_write 调用填充 HTTP 应答 
    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    static int m_epollfd;   // epoll 内核事件表的文件描述符
    static int m_user_count;    // 用户数量
    MYSQL *mysql;
    int m_state;  // 数据库的行为状态 读为0, 写为1

private:
    int m_sockfd;   // HTTP 连接的 socket (索引？？？)
    sockaddr_in m_address;  // 对方的 socket 地址 对方？？？

    char m_read_buf[READ_BUFFER_SIZE];  // 读缓冲区
    int m_read_idx; // 缓冲区中已读数据最后一个字节的 下一个位置
    int m_checked_idx;  // 当前分析字符的位置
    int m_start_line;   // 当前解析的行的 起始位置
    char m_write_buf[WRITE_BUFFER_SIZE];    // 写 缓冲区
    int m_write_idx;    // 待发送的 字节数

    CHECK_STATE m_check_state; 
    METHOD m_method;

    char m_real_file[FILENAME_LEN]; // 请求文件的完整路径
    char *m_url;    // 文件名
    char *m_version;    // HTTP 版本号
    char *m_host;   // 主机名
    int m_content_length;   // 消息体的长度
    bool m_linger;  // HTTP 请求是否保持连接

    char *m_file_address; // 目标文件映射(mmap)到内存中的 起始位置
    struct stat m_file_stat;    // 文件的状态
    struct iovec m_iv[2];   
    int m_iv_count; // 内存块的数量

    int cgi;         //是否启用的POST
    char *m_string;   //存储请求头数据
    int bytes_to_send;
    int bytes_have_send;
    char *doc_root;

    map<string, string> m_users;
    int m_TRIGMode;
    int m_close_log;

    char sql_user[100]; // 数据库 用户名称
    char sql_passwd[100];   // 数据库 密码
    char sql_name[100]; // 数据库 中 database 的名称
};

#endif
