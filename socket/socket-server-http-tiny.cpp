// #define _BSD_SOURCE
#include <memory>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <string>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <set>
#include <deque>
#include <list>
#include <utility>
#include <signal.h>
#include <vector>
#include <map>
#include <sstream>

// inet_ntoa <=> inet_addr
// ntohs <=> htons

enum
{
    MSG_SIZE = 4096
};

pthread_mutex_t g_mtx = PTHREAD_MUTEX_INITIALIZER;

struct Header
{
    enum Method
    {
        GET,
        POST
    } method;

    std::string resource;
    std::string querystring;
    std::string httpversion;
    std::string host;
    std::vector<std::string> accepts;
};

struct HttpRequest
{
    Header header;
    std::string body;
};

/**
 * @brief 删除空白字符
 * 
 * @param strSource 字符串
 * @param bEnd true 两端  false 所有
 */
void strip(std::string &strSource, bool bEnd = true)
{
    std::string result;
    result.reserve(strSource.size());

    if (bEnd)
    {
        int start = -1, end = -1;
        for (size_t i = 0; i < strSource.size() != 0; ++i)
        {
            char ch = strSource.at(i);
            if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t')
            {
                continue;
            }
            else if (start == -1)
            {
                start = i;
            }
            else
            {
                end = i;
            }
        }
        start = start == -1 ? 0 : start;
        strSource = strSource.substr(start, end - start + 1);
    }
    else
    {
        for (size_t i = 0; i < strSource.size() != 0; ++i)
        {
            char ch = strSource.at(i);
            if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t')
            {
                continue;
            }
            result.push_back(ch);
        }
        strSource = result;
    }
}

size_t splitstr(std::vector<std::string> &vecResults, const std::string &strSource, const std::string &strSep, int nMatch = -1)
{
    size_t poslast = 0;
    std::deque<std::string::value_type> queCurVal;
    for (size_t i = 0; i < strSource.size() && nMatch != 0; ++i)
    {
        if (queCurVal.size() < strSep.size())
            queCurVal.push_back(strSource.at(i));
        else
        {
            if ((std::string(queCurVal.begin(), queCurVal.end()) == strSep))
            {
                vecResults.push_back(strSource.substr(poslast, i - poslast - strSep.size()));
                poslast = i;
                if (nMatch > 0)
                    --nMatch;
            }
            queCurVal.pop_front();
            queCurVal.push_back(strSource.at(i));
        }
    }

    vecResults.push_back(strSource.substr(poslast, strSource.size()  - poslast));
    return vecResults.size();
}

bool parse_http_request_header(const std::string &strHead, Header &header)
{
    /**
     * 请求行(方法 url http版本)
     * 请求首部字段
     * 通用首部字段
     * 实体首部字段
     * 其他
     */
    std::vector<std::string> http_request_head;
    if (splitstr(http_request_head, strHead, "\r\n") < 3)
    {
        return false;
    }

    std::vector<std::string> request_info;
    if (splitstr(request_info, http_request_head.at(0), " ") < 3)
    {
        return false;
    }

    if (strncasecmp(request_info.at(0).c_str(), "GET", 3) == 0)
    {
        header.method = Header::GET;
    }
    else if (strncasecmp(request_info.at(0).c_str(), "POST", 4) == 0)
    {
        header.method = Header::POST;
    }

    if (header.method == Header::GET){
        std::vector<std::string> vecQuerys;
        splitstr(vecQuerys, request_info.at(1), "?", 1);
        if (vecQuerys.size() < 1){
            return false;
        }else if (vecQuerys.size() > 1){
            header.querystring = vecQuerys.at(1);
        }
        header.resource = vecQuerys.at(0);
    }
    header.httpversion = request_info.at(2);

    std::map<std::string, std::string> mapHeader;
    for (size_t i = 1; i < http_request_head.size(); ++i)
    {
        auto &val = http_request_head.at(i);
        strip(val);
        if (val.empty())
            continue;
        std::vector<std::string> pair;
        splitstr(pair, val, ":", 1);
        if (pair.size() < 2)
        {
            return false;
        }
        strip(pair.at(0));
        strip(pair.at(1));
        mapHeader.insert(std::make_pair(pair.at(0), pair.at(1)));
    }

    header.host = mapHeader["Host"];
    // header.accepts = mapHeader["Accept"];
    return true;
}

bool parse_http_request(const std::string &strHead, HttpRequest &httpReq)
{
    /**
     * 报文首部
     * 空行
     * 报文主体
     */
    std::vector<std::string> http_request;
    const size_t nLen = splitstr(http_request, strHead, "\r\n\r\n");
    if (nLen < 1)
    {
        return false;
    }

    if (nLen == 2)
    {
        // 包含body
        httpReq.body = http_request.at(1);
    }

    return parse_http_request_header(http_request.at(0), httpReq.header);
}

std::string GetCodeDesc(int status)
{
    switch (status)
    {
    case 200:
        return "Ok";
    case 400:
        return "Bad Request";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 500:
        return "Internal Server Error";
    case 501:
        return "Not Implemented";
    default:
        break;
    }

    return "Not Implemented";
}

std::string __make_http_response(uint32_t status, const std::string &content_type, const std::string &strContent)
{
    std::ostringstream oss;
    oss << "HTTP/1.1 ";
    oss << status;
    oss << " ";
    oss << GetCodeDesc(status);
    oss << "\r\n";
    oss << "Server: TinyHttp/1.1";
    oss << "\r\n";
    oss << "Content-Type: ";
    oss << content_type;
    oss << "\r\n";
    oss << "Content-Length: ";
    oss << strContent.size();
    oss << "\r\n";
    oss << "\r\n";
    oss << strContent;
    return oss.str();
}


#include <fstream>
std::string make_http_response(const HttpRequest &httpReq, const std::string &remotehost)
{
    char szContent[512] = {0};
    uint32_t code = 200;
    if (httpReq.header.method == Header::GET)
    {
        if (httpReq.header.resource == "/")
        {

            snprintf(szContent, sizeof(szContent),
                    "<HTML><TITLE>欢迎使用TinyHttp</TITLE><BODY><H1>亲爱的%s, 欢迎使用TinyHttp</H1><P>这是一个迷你HTTP服务</P></BODY></HTML>", remotehost.c_str());

        }else{
            char resourc_path[256] ={0};
            snprintf(resourc_path, sizeof(resourc_path), "bin/static%s", httpReq.header.resource.c_str());
            std::ifstream ifs(resourc_path);
            if (!ifs.is_open())
            {         
                code = 404;
                snprintf(szContent, sizeof(szContent),
                    "<HTML><TITLE>欢迎使用TinyHttp</TITLE><BODY><H1>Not Found</H1></BODY></HTML>", remotehost.c_str());
            }else{
                ifs.read(szContent, sizeof(szContent));
                ifs.close();
            }
            
        }
    }else{
        
    }


    return __make_http_response(code, "Text/Html; charset=UTF-8", szContent);
}

std::string get_remote_host(int sockfd)
{
    char szHost[21] = {0};
    struct sockaddr_in sa;
    socklen_t len = sizeof(sa);
    if (!getpeername(sockfd, (struct sockaddr *)&sa, &len))
    {
        snprintf(szHost, sizeof(szHost), "%s:%d", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
    }
    return szHost;
}

void *_thread_func(void *pData)
{
    std::set<int> *pSetFdAccepted = (std::set<int> *)(pData);
    std::list<std::pair<int, std::string>> listMsgs;
    for (;;)
    {
        if (pSetFdAccepted->empty()){
            sleep(0);
            continue;
        }

        fd_set setrd, setwd;
        FD_ZERO(&setrd);
        FD_ZERO(&setwd);

        int maxfd = 0;
        for (auto iter = pSetFdAccepted->begin(); iter != pSetFdAccepted->end(); ++iter)
        {
            FD_SET(*iter, &setrd);
            FD_SET(*iter, &setwd);
            if (*iter > maxfd)
            {
                maxfd = *iter + 1;
            }
        }

        int ret = select(maxfd, &setrd, &setwd, NULL, NULL);
        if (ret <= 0)
        {
            continue;
        }

        pthread_mutex_lock(&g_mtx);
        for (auto iter = pSetFdAccepted->begin(); iter != pSetFdAccepted->end();)
        {
            bool bClosed = false;
            if (FD_ISSET(*iter, &setwd))
            {
                for (auto listIter = listMsgs.begin(); listIter != listMsgs.end();)
                {
                    if (listIter->first != *iter)
                    {
                        ++listIter;
                        continue;
                    }

                    auto msg = listIter->second.c_str();
                    ssize_t _n_read = send(listIter->first, msg, strlen(msg), 0);
                    if (_n_read == -1)
                    {
                        fprintf(stderr, "Error: send failed: %s\n", strerror(errno));
                        break;
                    }

                    // fprintf(stderr, "send %d bytes msg: %s\n", _n_read, msg);
                    bClosed = true;
                    listIter = listMsgs.erase(listIter);
                }
            }

            if (FD_ISSET(*iter, &setrd))
            {
                char msg[MSG_SIZE] = {0};
                ssize_t _n_read = recv(*iter, msg, sizeof(msg), 0);
                if (_n_read <= 0)
                {
                    fprintf(stderr, "Error: recv failed: %s\n", strerror(errno));
                    bClosed = true;
                }
                else
                {
                    fprintf(stderr, "recv msg %d bytes: %s\n", _n_read, msg);

                    HttpRequest httpReq;
                    parse_http_request(msg, httpReq);
                    const auto response = make_http_response(httpReq, get_remote_host(*iter));
                    listMsgs.push_back(std::make_pair(*iter, response));
                }
            }

            if (bClosed)
            {
                close(*iter);
                iter = pSetFdAccepted->erase(iter);
            }
            else
                ++iter;
        }
        pthread_mutex_unlock(&g_mtx);
    }

    fprintf(stderr, "\n[%lu] goodbye\n", pthread_self());
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: socket-server-tcp-select [port] [backlog:options,default 4]\n");
        return EXIT_FAILURE;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        return EXIT_FAILURE;
    }

    // 设置端口复用， 避免重启服务显示端口占用情况发生
    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));

    struct sockaddr_in _addr;
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(atoi(argv[1]));
    _addr.sin_addr.s_addr = INADDR_ANY; // inet_addr(argv[1]);
    if (-1 == bind(sock, (struct sockaddr *)&_addr, sizeof(_addr)))
    {
        fprintf(stderr, "Error: bind failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    const int backlog = (argc == 3) ? atoi(argv[2]) : 4;
    if (-1 == listen(sock, backlog))
    {
        fprintf(stderr, "Error: listen failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // 忽略SIGPIPE信号， 避免对端socket关闭后， 触发此信号导致进程终止
    signal(SIGPIPE, SIG_IGN);

    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
    fprintf(stderr, "listen to: 0.0.0.0:%s\n", argv[1]);

    std::set<int> setFdAccepted;

    pthread_t thread;
    if (0 > pthread_create(&thread, NULL, _thread_func, &setFdAccepted))
    {
        fprintf(stderr, "Error: pthread_create failed: %s\n", strerror(errno));
    }
    else
    {
        for (;;)
        {
            struct sockaddr_in addr_client;
            socklen_t _n_addr_client = 0;
            const int sock_client = accept(sock, (struct sockaddr *)&addr_client, &_n_addr_client);
            if (-1 == sock_client)
            {
                continue;
            }

            fcntl(sock_client, F_SETFL, fcntl(sock_client, F_GETFL) | O_NONBLOCK);
            pthread_mutex_lock(&g_mtx);
            setFdAccepted.insert(sock_client);
            pthread_mutex_unlock(&g_mtx);

            fprintf(stderr, "accept %s connect\n", inet_ntoa(addr_client.sin_addr));
        }
        pthread_detach(thread);
    }
    close(sock);

    return EXIT_SUCCESS;
}
