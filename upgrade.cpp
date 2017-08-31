#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/time.h>
#include "upgrade.h"
#include "upload.h"
#include "log.h"
#define url "http://172.20.19.114:8080/upgrade/sleep_upgrade.tar.gz"
#define domain "172.20.19.114"
#define ip_addr "172.20.19.114"
#define port 8080
#define file_name "sleep_upgrade.tar.gz"
extern log_st *log_sleep;
struct resp_header resp;//全变量以便在多个进程中使用
#if 0
void parse_url(const char *url, char *domain, int *port, char *file_name)
{
    /*通过url解析出域名, 端口, 以及文件名*/
    int j = 0;
    int start = 0;
    *port = 80;
    char *patterns[] = {"http://", "https://", NULL};

    for (int i = 0; patterns[i]; i++)
        if (strncmp(url, patterns[i], strlen(patterns[i])) == 0)
            start = strlen(patterns[i]);

    //解析域名, 这里处理时域名后面的端口号会保留
    for (int i = start; url[i] != '/' && url[i] != '\0'; i++, j++)
        domain[j] = url[i];
    domain[j] = '\0';

    //解析端口号, 如果没有, 那么设置端口为80
    char *pos = strstr(domain, ":");
    if (pos)
        sscanf(pos, ":%d", port);

    //删除域名端口号

    for (int i = 0; i < (int)strlen(domain); i++)
    {
        if (domain[i] == ':')
        {
            domain[i] = '\0';
            break;
        }
    }

    //获取下载文件名
    j = 0;
    for (int i = start; url[i] != '\0'; i++)
    {
        if (url[i] == '/')
        {
            if (i !=  strlen(url) - 1)
                j = 0;
            continue;
        }
        else
            file_name[j++] = url[i];
    }
    file_name[j] = '\0';
}
#endif
struct resp_header get_resp_header(const char *response)
{
    /*获取响应头的信息*/
    struct resp_header resp;

    char *pos =(char *) strstr(response, "HTTP/");
    if (pos)
        sscanf(pos, "%*s %d", &resp.status_code);//返回状态码

    pos = (char *) strstr(response, "Content-Type:");//返回内容类型
    if (pos)
        sscanf(pos, "%*s %s", resp.content_type);

    pos = (char *) strstr(response, "Content-Length:");//内容的长度(字节)
    if (pos)
        sscanf(pos, "%*s %ld", &resp.content_length);

    return resp;
}

/*void get_ip_addr(char *domain, char *ip_addr)
{
    //通过域名得到相应的ip地址
    struct hostent *host = gethostbyname(domain);
    if (!host)
    {
        ip_addr = NULL;
        return;
    }

    for (int i = 0; host->h_addr_list[i]; i++)
    {
        strcpy(ip_addr, inet_ntoa( * (struct in_addr*) host->h_addr_list[i]));
        break;
    }
}*/


void progressBar(long cur_size, long total_size)
{
    /*用于显示下载进度条*/
    float percent = (float) cur_size / total_size;
    const int numTotal = 50;
    int numShow = (int)(numTotal * percent);

    if (numShow == 0)
        numShow = 1;

    if (numShow > numTotal)
        numShow = numTotal;

    char sign[51] = {0};
    memset(sign, '=', numTotal);

    DBG("\r%.2f%%\t[%-*.*s] %.2f/%.2fMB\r\n", percent * 100, numTotal, numShow, sign, cur_size / 1024.0 / 1024.0, total_size / 1024.0 / 1024.0);
    fflush(stdout);

    if (numShow == numTotal)
        DBG("\n");
}

void * download(void * socket_d)
{
    /*下载文件函数, 放在线程中执行*/
    int client_socket = *(int *) socket_d;
    int length = 0;
    int mem_size = 4096;//mem_size might be enlarge, so reset it
    int buf_len = mem_size;//read 4k each time
    int len;

    //创建文件描述符
    int fd = open(file_name, O_CREAT | O_WRONLY, S_IRWXG | S_IRWXO | S_IRWXU);
    if (fd < 0)
    {
        DBG("upgrade: Create file failed\n");
        LogDebug(log_sleep, "upgrade: Create file failed\n");
        exit(0);
    }

    char *buf = (char *) malloc(mem_size * sizeof(char));

    //从套接字中读取文件流
    while ((len = read(client_socket, buf, buf_len)) != 0 && length < resp.content_length)
    {
        write(fd, buf, len);
        length += len;
        progressBar(length, resp.content_length);
    }

    if (length == resp.content_length)
    {
    	DBG("Download successful ^_^\n\n");
    	send_update(Device_ID,0);
    	//system("mv -f sleep_upgrade sleep_detect &");
    	system("mkdir ./sleep_upgrade");
    	sleep(1);
    	system("tar -zxvf sleep_upgrade.tar.gz -C ./sleep_upgrade");
    	sleep(1);
    	system("mv -f ./sleep_upgrade/rc.local /etc/");
    	sleep(1);
    	system("mv -f ./sleep_upgrade/* .");
    	sleep(1);
    	system("rm -rf ./sleep_upgrade");
    	sleep(1);
    	system("rm -f sleep_upgrade.tar.gz");
    	sleep(1);
    	system("reboot &");
    	close(fd);
    }

}

int UpgradeApp()
{
    /* test url: 1. https://nodejs.org/dist/v4.2.3/node-v4.2.3-linux-x64.tar.gz 2. http://img.ivsky.com/img/tupian/pre/201312/04/nelumbo_nucifera-009.jpg */
    /* char url[2048] = "127.0.0.1";
    char domain[64] = {0};
    char ip_addr[16] = {0};
    int port = 80;
    char file_name[256] = {0};

    if (argc == 1)
    {
        DBG("Input a valid URL please\n");
        exit(0);
    }
    else
        strcpy(url, argv[1]);

    puts("1: Parsing url...");
    parse_url(url, domain, &port, file_name);

    if (argc == 3)
        strcpy(file_name, argv[2]);

    puts("2: Get ip address...");
    get_ip_addr(domain, ip_addr);
    if (strlen(ip_addr) == 0)
    {
        DBG("can not get ip address\n");
        return 0;
    }

    puts("\n>>>>Detail<<<<");
    DBG("URL: %s\n", url);
    DBG("DOMAIN: %s\n", domain);
    DBG("IP: %s\n", ip_addr);
    DBG("PORT: %d\n", port);
    DBG("FILENAME: %s\n\n", file_name);
*/
    //设置http请求头信息
    char header[2048] = {0};
    sprintf(header, \
            "GET %s HTTP/1.1\r\n"\
            "Accept:text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"\
            "User-Agent:Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537(KHTML, like Gecko) Chrome/47.0.2526Safari/537.36\r\n"\
            "Host:%s\r\n"\
            "Connection:close\r\n"\
            "\r\n"\
        ,url, domain);

    //DBG("%s\n%d", header, (int) strlen(header));

    //创建套接字
    int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket < 0)
    {
        DBG("upgrade: invalid socket descriptor: %d\r\n", client_socket);
        LogDebug(log_sleep, "upgrade: invalid socket descriptor: %d\r\n", client_socket);
        //exit(-1);
        send_update(Device_ID,1);
        return -1;
    }

    //创建地址结构体
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_addr);
    addr.sin_port = htons(port);

    //连接服务器
    DBG("3: Connect server...\r\n");
    int res = connect(client_socket, (struct sockaddr *) &addr, sizeof(addr));
    if (res == -1)
    {
        DBG("upgrade: connect failed, return: %d\r\n", res);
        LogDebug(log_sleep, "upgrade: connect failed, return: %d\r\n", res);
        //exit(-1);
        send_update(Device_ID,1);
        return -1;
    }

    DBG("4: Send request...\r\n");//向服务器发送下载请求
    write(client_socket, header, strlen(header));

    int mem_size = 4096;
    int length = 0;
    int len;
    char *buf = (char *) malloc(mem_size * sizeof(char));
    char *response = (char *) malloc(mem_size * sizeof(char));

    //每次单个字符读取响应头信息, 仅仅读取的是响应部分的头部, 后面单独开线程下载
    while ((len = read(client_socket, buf, 1)) != 0)
    {
        if (length + len > mem_size)
        {
            //动态内存申请, 因为无法确定响应头内容长度
            mem_size *= 2;
            char * temp = (char *) realloc(response, sizeof(char) * mem_size);
            if (temp == NULL)
            {
                DBG("realloc failed\r\n");
                //exit(-1);
                send_update(Device_ID,1);
                return -1;
            }
            response = temp;
        }

        buf[len] = '\0';
        strcat(response, buf);

        //找到响应头的头部信息, 两个"\n\r"为分割点
        int flag = 0;
        for (int i = strlen(response) - 1; response[i] == '\n' || response[i] == '\r'; i--, flag++);
        if (flag == 4)
            break;

        length += len;
    }

    //DBG("\r\n>>>>Response header:<<<<\r\n%s", response);

    resp = get_resp_header(response);
    //strcpy(resp.file_name, file_name);

    DBG("5: Start thread to download...\r\n");
    /*开新的线程下载文件*/
    pthread_t download_thread;
    pthread_create(&download_thread, NULL, download, (void *) &client_socket);
    pthread_join(download_thread, NULL);
    return 0;
}
