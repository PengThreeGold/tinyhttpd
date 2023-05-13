// 项目名称：基于HTTP协议服务器用于改变网页背景颜色
// creattime: 2023/05/07

/* HTTP介绍
    HTTP协议介绍：
    超文本传输协议（HyperText Transfer Protocol）,是应用层的通信协议。
    通常由HTTP客户端发起一个请求，创建一个到服务器指定端口（默认80端口）的TCP连接。
    HTTP服务器则在端口监听客户端请求。
    一旦接收到请求，服务器会向客户端返回一个状态，例如“HTTP/1.1 200 OK”，以及返回的内容，如请求的文件。

    HTTP服务器工作原理：

    1、客户端连接到Web服务器。
        一个HTTP客户端（通常是浏览器）与Web服务器的HTTP端口（80端口）建立一个TCP套接字连接

    2、发送HTTP请求。
        通过TCP套接字，客户端向Web服务器发送一个文本的请求报文，请求报文格式如下：
            请求行\r\n
            请求头部\r\n
            \r\n
            请求数据\r\n

    3、服务器接受请求并返回HTTP响应。
        服务端read套接字，解析请求文本，定位请求资源。
        服务器将资源write到TCP套接字，由客户端读取，响应报本格式如下：
            状态行\r\n
            响应头部\r\n
            \r\n
            响应数据

    4、客户端浏览器解析HTML内容。
        客户端read套接字，解析响应文本，查看状态码并处理和显示信息。

    5、断开TCP连接（请求头 connection=close）。

    6、HTTP请求格式（请求协议）。
        |请求方法|空格符|ULR|空格符|协议版本|回车符|换行符|
        |头部字段名|:|值|回车符|换行符|
        ……
        |头部字段名|:|值|回车符|换行符|
        |回车符|换行符|
        …… （此为请求数据）

        例如：
            POST /htdocs/index.html HTTP/1.1 \r\n
            Host: www.example.com            \r\n
            content-Length: 9                \r\n
            \r\n
            color=red

    7、HTTP响应格式（响应协议）。
        |协议版本|空格符|状态码|空格符|状态码文字描述|回车符|换行符|
        |头部字段名|:|值|回车符|换行符|
        ……
        |头部字段名|:|值|回车符|换行符|
        |回车符|换行符|
        …… （此为响应数据）

        例如：
            HTTP/1.1 200 OK          \r\n
            content-type: text/html  \r\n
            content-Length: 9        \r\n
            \r\n
            <html>
            ……

    8、状态码（部分）。
        400：客户端请求的语法错误，服务器无法解析导致解析错误。
        404：服务器无法根据客户端请求定向资源。
        500：服务器内部报障。
        501：服务器不支持该请求功能，无法完成请求操作（post/get/……）。
*/

/* 要为Linux进行编译，请执行以下操作：
 * 1）注释掉 #include <pthread.h> 行；
 * 2）注释掉定义变量 newthread 的行；
 * 3）注释掉运行 pthread_create() 的两行；
 * 4）取消注释运行 accept_request() 的行；
 * 5）从 MakeFike 中删除-lsocket;
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
// #include <pthread.h>

// 检测是否为空格
#define ISspace(x) (x == ' ')
// 默认服务器字符串提示
#define SERVER_STRING "Server: jdbhttpd/0.1.0\r\n"

int startup(unsigned short *); // 初始化服务器，包括建立套接字，绑定端口，进行监听等。
void *accept_request(int); // 处理从套接字上监听到的一个 HTTP 请求，包含服务器处理请求基本流程。
void execute_cgi(int, const char *, const char *, const char *); //用于执行CGI脚本程序。
void serve_file(int, const char *); // 处理相关服务器文件（调用cat函数等）并返回给浏览器。
int get_line(int, char *, int); // 读取套接字的一行。
void cat(int, FILE *); // 处理服务器文件，读取文件内容写入到套接字发送到客户端。
void headers(int, const char *); // 把HTTP响应头部写入套接字，服务器成功响应返回200
void bad_request(int); // 400错误处理函数-服务器无法解析
void not_found(int); // 404错误处理函数-请求的内容不存在
void cannot_execute(int); // 500错误处理函数-无法执行请求（如CGI程序）
void unimplemented(int); // 501（仅实现了get、post方法）错误处理函数-请求方法错误
void error_die(const char *); // 错误处理函数处理

// **********************************************************************

int main(int argc, char *argv[])
{
    int server_sock = -1; // 定义服务器socket描述符
    unsigned short port = 8888; // 定义服务端监听端口
    int client_sock = -1; // 定义客户端socket描述符
    struct sockaddr_in client_name; // 定义sockaddr_int 类型结构体，accept阶段用来获取客户端的IPV4地址信息
    socklen_t client_name_len = sizeof(client_name); // 获取客户端地址长度

    // pthread_t newthread; // 定义线程id（在Linux平台上需要注释掉）

    server_sock = startup(&port);               // 初始化服务器
    printf("HTTP SERVER IS RUNING !!!\nhttpd running on port %d\n", port); // 输出当前端口号

    // 循环创建链接和子线程
    while (1)
    {
        // 阻塞等待客户端建立链接
        client_sock = accept(server_sock,
                             (struct sockaddr *)&client_name,
                             &client_name_len); 

        if (client_sock < 0)
        {
            error_die("accept");
        }
        accept_request(client_sock);
        printf("log------clientport:%d\n", ntohl(client_name.sin_port)); // 测试打印客户端端口
        // 创建子线程处理链接（在Linux平台上需要注释掉）
        // if (pthread_create(&newthread, NULL, (void *)accept_request, (void *)(intptr_t)client_sock) != 0)
        // {
        //     perror("pthread_create");
        // }
    }
    close(server_sock);
    return 0;
}

/// @brief 启动监听web连接程序函数，包括建立套接字，绑定端口，进行监听等。
/// @param port 可用连接端口的指针，若端口为0则自动指定可用端口
/// @return the socket socket描述符
int startup(unsigned short *port)
{
    // 定义服务器socket描述符
    int httpd = 0;
    // int on = 1;
    // 用来绑定（初始化）服务端的ip地址和端口
    struct sockaddr_in name;
    /* 创建服务器端
     * socket_PF_INET 地址类型（默认为ipv4）
     * SOCK_STREAM socket的类型（默认为tcp）
     * 参数0即表示上述类型为默认协议
     */
    httpd = socket(PF_INET, SOCK_STREAM, 0);
    // 错误判断
    if (httpd == -1)
    {
        error_die("socket");
    }
    // 把结构体里每个字节都填充为0，以用于初始化结构体
    memset(&name, 0, sizeof(name));
    // 地址类型为IPV4
    name.sin_family = AF_INET;
    // 端口转换为网络字节序（大端存储）
    name.sin_port = htons(*port);
    // 本机任一可用ip地址(宏定义)。INADDR_ANY是一个IPV4通配地址常量
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // 手动指定ip地址
    // inet_pton(AF_INET, "192.168.1.1", &name.sin_addr.s_addr);

    // if ((setsockopt(httpd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)
    // {
    //     error_die("setsockopt failed");
    // }

    // 绑定地址与socket
    if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
    {
        error_die("bind");
    }
    // 传递的端口号如果是0，则会手动调用getsockname()选取可用端口
    if (*port == 0)
    {
        socklen_t namelen = sizeof(name);
        // 获取已经绑定的套接字的地址信息（主要获取端口号）
        if (getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1)
            error_die("getsockname");
        *port = ntohs(name.sin_port); // 转换为主机字节序（小端存储）
    }

    // 服务器开始监听。（最初的 BSD socket 实现中，backlog 的上限是5）
    if (listen(httpd, 5) < 0)
    {
        error_die("listen");
    }

    // 返回服务器scoket描述符
    return (httpd);
}

/// @brief 接受服务器端口请求根据需求进行处理。
/// @param arg 链接到客户端的套接字
/// @return 适当地处理请求
void *accept_request(int arg)
{
    // 缓冲区
    char buf[1024];
    // 数据长度
    int numbers;
    // 标识变量
    size_t i, j;
    // 请求方法
    char method[255];
    // 标志是否调用CGI程序
    int cgi = 0;
    // 存放url
    char url[255];
    char *query_string = NULL;
    // 路径
    char path[512];
    // 文件状态信息
    struct stat st;
    // 建立链接的socket描述符
    int client = (intptr_t)arg;

    // detach 子线程分离（Linux平台注释掉）
    // pthread_detach(pthread_self());

    // 服务器读取行数据
    numbers = get_line(client, buf, sizeof(buf));

    i = 0;
    j = 0;
    // 根据空格定位方法（为空格且小于缓冲区大小）
    while (!ISspace(buf[i]) && (i < sizeof(method) - 1))
    {
        method[i] = buf[i];
        i++;
    }
    j = i;
    // 请求方法字符串结尾标识符
    method[i] = '\0';

    // 测试：打印方法
    printf("test----------%s\n", method);

    // 判断是否为GET与POST操作
    /* strcasecmp() 若参数s1和s2字符串相等则返回0。
     * s1大于s2则返回大于0 的值，s1 小于s2 则返回小于0的值
     */
    if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))
    {
        unimplemented(client);
        return NULL;
    }
    // 判断是否为POST方法
    if (strcasecmp(method, "POST") == 0)
    {
        // 标识调用CGI
        cgi = 1;
    }
    // 提取url

    i = 0;
    // 指向url开始处(跳过请求方法与url之间的空格)
    while (ISspace(buf[j]) && (i < sizeof(buf)))
    {
        j++;
    }
    // 判断 不为空&&小于url存放区&&小于总数据报长
    while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf)))
    {
        // 传递url给url存放区
        url[i] = buf[j];
        i++;
        j++;
    }

    // url字符串结尾标识符
    url[i] = '\0';

    // 如果是get方法通过url中是否包含？确定是否包含查询数据
    if (strcasecmp(method, "GET") == 0)
    {
        // 将url（首字母）地址赋值给查询数据指针（char *型）
        query_string = url;
        // 查询数据当前不等于'?' && 查询数据未到结尾
        while ((*query_string != '?') && (*query_string != '\0'))
        {
            // 向下遍历查询数据指针
            query_string++;
        }
        if (*query_string == '?') // 如果找到某个字符为'?'
        {
            cgi = 1;              // 标识需要程序处理CGI
            *query_string = '\0'; // 令当前字符（'?'）为0
            query_string++;       // 指向0（'?'）后面的请求内容
        }
    }

    // 如果url是一个目录的话，就用该目录下index.html文件返回
    sprintf(path, "htdocs%s", url);    // 将 htdocs 与 url 拼接到 path
    if (path[strlen(path) - 1] == '/') // 判断路径最后字符是否为目录
    {
        strcat(path, "index.html"); // 拼接路径与首页（index.html）
    }

    // 测试输出
    // printf("log------path:%s\n", path);

    if (stat(path, &st) == -1) // 判断文件是否存在
    {
        // 不存在
        while ((numbers > 0) && strcmp("\n", buf)) // 读取并丢弃headers
        {
            numbers = get_line(client, buf, sizeof(buf));
        }
        // 执行文件未找到程序
        not_found(client);
    }

    else // 文件存在
    {
        // if ((st.st_mode & S_IFMT) == S_IFDIR) // 现代编译器默认隐藏并将此定义为宏函数S_ISDIR()
        if (S_ISDIR(st.st_mode)) // 判断文件是否为目录
        {
            strcat(path, "/index.html"); // 拼接路径和主页
        }
        if ((st.st_mode & S_IXUSR) ||
            (st.st_mode & S_IXGRP) ||
            (st.st_mode & S_IXOTH)) // 判断是否拥有执行权限（可执行程序）
        {
            cgi = 1; // 标识CGI程序标识符
        }
        if (!cgi) // 判断是否要调用CGI程序
        {
            serve_file(client, path); // 请求了一个文件，执行serve_file()
        }
        else
        {
            execute_cgi(client, path, method, query_string); // 需要调用CGI，执行execute_cgi()
        }
    }
    close(client); // 关闭客户端
    return NULL;
}

/// @brief 执行CGI脚本，需要根据需求设置环境变量。
/// @param client 套接字描述符
/// @param path 请求的CGI文件路径
/// @param method 请求的方法
/// @param query_string 请求数据
void execute_cgi(int client,
                 const char *path,
                 const char *method,
                 const char *query_string)
{
    char buf[1024]; // 缓冲池
    int cgi_output[2];
    int cgi_input[2];
    pid_t pid; // 进程id
    int status; // 状态
    int i;
    char c;
    int numchars = 1;
    int content_length = -1;
    // 字符填充保证能够通过下面的while()判别
    buf[0] = 'A';
    buf[1] = '\0';

    if (strcasecmp(method, "GET") == 0) // 判断是否为GET方法
    {
        // 读取并丢弃头部信息
        while ((numchars > 0) && strcmp("\n", buf))
        {
            numchars = get_line(client, buf, sizeof(buf));
        }
    }
    else if (strcasecmp(method, "POST") == 0) // 判断是否为POST方法
    {
        // 查询头部content-length信息
        // 获取与body参数字符数相等的行
        numchars = get_line(client, buf, sizeof(buf));
        while ((numchars > 0) && strcmp("\n", buf))
        {

            // 比较前15（0-14）个字符，利用'\0'在第15位进行截止
            buf[15] = '\0';
            if (strcasecmp(buf, "Content-Length:") == 0)
            {
                // 将后面的字符转换为int类型数据
                content_length = atoi(&(buf[16]));
                // break;
            }
            numchars = get_line(client, buf, sizeof(buf));// 逐行读取
        }
        if (content_length == -1)  // 错误处理
        {
            bad_request(client);
            return;
        }
    }


    sprintf(buf, "HTTP/1.0 200 OK\r\n"); // 拼接服务器响应头部
    send(client, buf, strlen(buf), 0); // 向服务器发送头部信息

    // ------创建两个管道用于进程间（子进程）通讯------
    // ----------------------------------------------
    
    if (pipe(cgi_output) < 0) // 子进程写管道
    {
        cannot_execute(client);
        return;
    }

    if (pipe(cgi_input) < 0) // 子进程读管道
    {
        cannot_execute(client);
        return;
    }

    if ((pid = fork()) < 0) // 创建子进程
    {
        cannot_execute(client);
        return;
    }

    if (pid == 0) // 子进程执行CGI脚本程序
    {
        char meth_env[255];
        char query_env[255];
        char length_env[255];

        // 将子进程的输出由标准输出重定向到cgi_output的管道写端上
        dup2(cgi_output[1], 1);
        // 将子进程的输入由标准输入重定向到cgi_output的管道读端上
        dup2(cgi_input[0], 0);
        
        close(cgi_output[0]); // 关闭不需要的写端
        close(cgi_input[1]); // 关闭不需要的读端

        // 通过环境变量传递信息，用于父子进程读写信息，实现进程间通信

        // 将请求方式创建键值对添加到环境变量中
        sprintf(meth_env, "REQUEST_METHOD=%s", method);
        putenv(meth_env); // 写入环境变量

        if (strcasecmp(method, "GET") == 0) // GET请求
        {
            // 如果是GET请求，创建键值对将请求内容添加到环境变量中
            sprintf(query_env, "QUERY_STRING=%s", query_string);
            putenv(query_env);
        }
        else // POST请求
        {
            // 如果是POST请求，创建键值对将请求内容添加到环境变量中
            sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
            putenv(length_env);
        }
        // 将子进程替换成另一个进程并执行脚本
        execl(path, path, NULL);
        exit(0);
    }
    else // 父进程
    {
        // 父进程关闭不需要的读写端（cgi_output&cgi_input）
        close(cgi_output[1]);
        close(cgi_input[0]);

        // 读取从客户端请求数据发送给CGI脚本程序
        if (strcasecmp(method, "POST") == 0) // 如果是POST方法
        {
            for (i = 0; i < content_length; i++)
            {
                recv(client, &c, 1, 0); // 服务器从客户端逐个字符读取
                fprintf(stderr, "%c\n", c); // 测试
                // 将字符写入gi_input管道由子进程读取（CGI程序根据环境变量获取长度）
                write(cgi_input[1], &c, 1);
            }
        }

        // 从cgi_output管道中逐个读取子进程的输出数据并发送给客户端
        while (read(cgi_output[0], &c, 1) > 0)
        {
            send(client, &c, 1, 0);
            fprintf(stderr, "%c", c);// 测试
        }
        // 关闭不需要的读写端
        close(cgi_input[1]);
        close(cgi_output[0]);

        waitpid(pid, &status, 0); // 等待子进程结束
    }
}

/// @brief 处理相关服务器文件（调用cat函数等）并返回给浏览器，包括用户头文件和报告。
/// @param client socket描述符
/// @param filename 请求的文件名称（路径
void serve_file(int client, const char *filename)
{
    // 文件资源指针
    FILE *resource = NULL;
    // 文件大小标志
    int numchars = 1;
    // 缓存池
    char buf[1024];

    // 字符填充保证能够通过下面的while()判别
    buf[0] = 'A';
    buf[1] = '\0';
    // 读取并丢弃http头部信息（返回信息有效且仅含有"\n"）
    while ((numchars > 0) && strcmp("\n", buf))
    {
        numchars = get_line(client, buf, sizeof(buf));
    }

    printf("log------filepath:%s\n", filename); // 测试输出

    // 以只读方式打开文件
    resource = fopen(filename, "r");

    if (resource == NULL)
    {
        // 错误处理
        not_found(client);
    }
    else
    {
        // 请求正确的头文件并返回
        headers(client, filename);
        // 逐行将文件的内容发送到客户端
        cat(client, resource);
    }
    // 关闭文件资源
    fclose(resource);
}
/// @brief 读取套接字的一行，把回车换行等情况都统一为换行符结束。
/// @param sock sock套接字
/// @param buf 字符缓冲池
/// @param size 缓冲池大小
/// @return
int get_line(int sock, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;

    while ((i < size - 1) && (c != '\n'))
    {
        // 读取一个字符数据存放在c中
        n = recv(sock, &c, 1, 0);
        // printf("%02X\n", c); // 调试
        if (n > 0)
        {
            if (c == '\r')
            {
                n = recv(sock, &c, 1, MSG_PEEK);
                // printf("%02X\n", c); // 调试
                if ((n > 0) && (c == '\n'))
                    recv(sock, &c, 1, 0);
                else
                    c = '\n';
            }
            buf[i] = c;
            i++;
        }
        else
            c = '\n';
    }
    buf[i] = '\0';

    return (i);
}

/// @brief 把一个文件的全部内容放在一个套接字。
/// @param client socket描述符
/// @param resource 文件名（文件指针）
void cat(int client, FILE *resource)
{
    char buf[1024];                    // 缓冲池
    fgets(buf, sizeof(buf), resource); // 从文件描述符中逐行读取，遇到换行符/eof/error停止

    while (!feof(resource)) // 判断是否读取到文件结尾
    {
        // 将读取内容发送到客户端
        send(client, buf, strlen(buf), 0);
        // 读取下一行
        fgets(buf, sizeof(buf), resource);
    }
}

/// @brief 把HTTP响应头部写入套接字并发送，表示服务器成功响应返回200。
/// @param client socket描述符
/// @param filename 文件指针
void headers(int client, const char *filename)
{
    // 缓冲池
    char buf[1024];
    // 使用文件名确认文件类型（调用stat等操作）
    (void)filename;

    // 拷贝协议版本 状态码 状态码文字描述
    strcpy(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    // 拷贝文件资源内容类型
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    // 空行
    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
}

/// @brief 处理400错误，错误请求。
/// @param client
void bad_request(int client)
{
    char buf[1024];

    sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "<P>Your browser sent a bad request, ");
    send(client, buf, sizeof(buf), 0);
    sprintf(buf, "such as a POST without a Content-Length.\r\n");
    send(client, buf, sizeof(buf), 0);
}

/// @brief 处理404错误，无法获取请求内容。
/// @param client
void not_found(int client)
{
    char buf[1024];

    sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "your request because the resource specified\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "is unavailable or nonexistent.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
}

/// @brief 处理500错误，无法执行函数，主要处理发生在执行 cgi 程序时出现的错误。
/// @param client
void cannot_execute(int client)
{
    char buf[1024];

    sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
    send(client, buf, strlen(buf), 0);
}

/// @brief 处理501错误，返回浏览器表明请求资源未找到或请求方法不支持。
/// @param client
void unimplemented(int client)
{
    char buf[1024];

    sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</TITLE></HEAD>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
}

/// @brief 处理其他错误，并把错误信息写到 perror 并退出。
/// @param sc
void error_die(const char *sc)
{
    perror(sc);
    exit(1);
}
