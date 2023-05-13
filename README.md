# tinyhttpd —— 基于TinyHttpd开源项目的复写
+ 实现了Linux操作系统下（GCC 11）编译执行
+ 实现了C++语言的GCI脚本的编写并编译执行
## 项目文件说明
+ int startup(unsigned short *) 用于启动监听web连接程序函数
+ void *accept_request(int arg) 用于接收服务器端口的请求并进行相关处理跳转
+ void execute_cgi(...) 用于执行CGI脚本程序，程序需要根据已经设置的环境变量请求相关数据
+ void serve_file(int client, const char *filename) 用于发送规则的文件给客户端，包含头文件与报文
+ int get_line(int sock, char *buf, int size) 用于逐行读取套接字信息，以回车或换行当做结束符
+ void cat(int client, FILE *resource) 用于套接字获取文件内容
+ void headers(int client, const char *filename) 用于获取文件信息并添加到HTTP头文件信息
+ void bad_request(int client) 用于400错误请求处理，这是个错误请求
+ void not_found(int client) 用于404错误信息处理，无法获取请求
+ void cannot_execute(int client) 用于500错误异常处理，无法正常执行资源（如求情CGI脚本程序）
+ void unimplemented(int client) 用于501错误请求处理，无法正常请求资源或请求方法不支持
+ void error_die(const char *sc) 用于将错误信息流推送到perror并退出服务
