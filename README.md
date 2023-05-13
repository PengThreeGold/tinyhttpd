# tinyhttpd —— 基于TinyHttpd开源项目的复写
+ 实现了Linux操作系统下（GCC 11）编译执行
+ 实现了C++语言的GCI脚本的编写并编译执行
## 项目文件说明
+ httpd.c 主程序文件
+ httpd.out 主程序编译后文件
+ htdocs CGI脚本程序相关文件
  + color.cpp CGI脚本程序文件
  + color.cgi CGI脚本程序编译后文件
  + example_cgi.html CGI脚本程序页面效果示例文件
  + index.html 服务器启动主页文件
## 主要函数说明（color.c文件）
+ startup() 用于启动监听web连接程序函数
+ accept_request() 用于接收服务器端口的HTTP请求并进行相关处理跳转
+ execute_cgi() 用于执行CGI脚本程序，程序需要根据已经设置的环境变量请求相关数据
+ serve_file() 用于处理相关服务器文件并发送给客户端，包含头文件与报文
+ get_line() 用于读取套接字的一行信息，以回车或换行当做结束符
+ cat() 用于处理服务器文件，读取文件内容写入到套接字
+ headers() 用于200正确请求并返回HTTP正确报文
+ bad_request() 用于400错误请求处理，这是个错误请求
+ not_found() 用于404错误信息处理，无法获取请求
+ cannot_execute() 用于500错误异常处理，无法正常执行资源（如求情CGI脚本程序）
+ unimplemented() 用于501错误请求处理，无法正常请求资源或请求方法不支持
+ error_die() 用于将错误信息流推送到perror并退出服务
## 运行界面
+ 主页
![index](https://github.com/PengThreeGold/tinyhttpd/blob/master/index.png)
+ CGI页面
![cgi](https://github.com/PengThreeGold/tinyhttpd/blob/master/cgi.png)
