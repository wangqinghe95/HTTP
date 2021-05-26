#ifndef _HTTP_REQUEST_HH
#define _HTTP_REQUEST_HH

#include <string>
#include <vector>
#include <assert.h>
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <cstring>
#include <map>
#include <sstream>

const size_t kBufferSize = 4096;

class Buffer
{

public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 4096;

    explicit Buffer(size_t initialSize = kInitialSize)
                    : m_buffer(kCheapPrepend + initialSize)
                    , m_readerIndex(kCheapPrepend)
                    , m_writerIndex(kCheapPrepend)
    {
        assert(readableBytes() == 0);
        assert(wirteableBytes() == initialSize);
        assert(prependableBytes() == kCheapPrepend);
    }

    size_t readableBytes() const
    {
        return m_writerIndex - m_readerIndex;
    }

    size_t wirteableBytes() const
    {
        return m_buffer.size() - m_writerIndex;
    }

    size_t prependableBytes() const
    {
        return m_readerIndex;
    }

    const char* peek() const{
        return begin() + m_readerIndex;
    }

    char* beginWrite(){
        return begin() + m_writerIndex;
    }

    void hasWritten(size_t len){
        assert(len <= wirteableBytes());
        m_writerIndex += len;
    }

    void unwrite(size_t len){
        assert(len <= readableBytes());
        m_writerIndex -= len;
    }

    void retrieve(size_t len){
        assert(len <= readableBytes());
        if (len < readableBytes()){
            m_readerIndex += len;
        }
        else{
            retriveAll();
        }
    }

    void retriveAll(){
        m_readerIndex = kCheapPrepend;
        m_writerIndex = kCheapPrepend;
    }

private:
    char* begin()
    {
        return &*m_buffer.begin();
    }

    const char* begin() const
    {
        return &*m_buffer.begin();
    }

private:
    std::vector<char> m_buffer;
    size_t m_readerIndex;
    size_t m_writerIndex;
};

namespace sockets
{
    int createSocket(sa_family_t family);
    int connect(int sockfd, const struct sockaddr* addr);
    size_t read(int sockfd, void *buf, size_t count);
    size_t readv(int sockfd, const struct iovec* iov, int iovcnt);
    size_t write(int sockfd, const void *buf, size_t count);

    void close(int sockfd);

    void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
    int getSocketError(int sockfd);
    void delaySecond(int sec);
}

class InetAddress
{
private:
    struct sockaddr_in m_addr;
public:
    InetAddress(std::string ip, uint16_t port);

    explicit InetAddress(const struct sockaddr_in& addr)
                : m_addr(addr){}
    
    sa_family_t family() const {
        return m_addr.sin_family;
    }

    const struct sockaddr* getSockAddr() const{
        return (struct sockaddr*)(&m_addr);
    }

    uint32_t ipNetEndian() const;

    ~InetAddress();
};

class HttpUrl
{
private:
    std::string m_httpUrl;
    std::vector<std::string> m_smatch;
public:
    HttpUrl(std::string& httpUrl)
            : m_httpUrl(httpUrl)
            , m_smatch(detachHttpUrl())
    {
        std::cout << "URL: " << m_httpUrl << std::endl;
    }
    ~HttpUrl();

    enum HttpUrlMatch
    {
        URL = 0,
        HOST = 1,
        URI = 2,
    };

    //https://www.cnblogs.com/ailumiyana/p/9839170.html
    std::vector<std::string> detachHttpUrl() const
    {
        std::vector<std::string> v;
        std::string::size_type pos1, pos2;
        pos2 = m_httpUrl.find('/'); //6
        assert(std::string::npos != pos2);
        pos1 = pos2 + 2;    //8 w
        pos2 = m_httpUrl.find('/', pos1); //
        assert(std::string::npos != pos2);
        v.push_back(m_httpUrl);
        v.push_back(m_httpUrl.substr(pos1, pos2-pos1));
        v.push_back(m_httpUrl.substr(pos2+1));
        std::cout << 'v[0] = ' << v[0] << std::endl;
        std::cout << 'v[1] = ' << v[1] << std::endl;
        std::cout << 'v[2] = ' << v[2] << std::endl;
        return v;
    }

    bool HttpUrlToIp(const std::string &host, char* ip) const
    {
        struct hostent* phost = NULL;
        phost = gethostbyname(host.c_str());

        if (NULL == phost){
            std::cout << "HttpUrlToIp(): gethostbyname error : " << errno  << " : " << strerror(errno) << std::endl;
            return false;
        }

        inet_ntop(phost->h_addrtype, phost->h_addr, ip, 17);

        return true;
    }

    std::string domain() const
    {
        return getHttpUrlSubSeg(HOST);
    }

    std::string getHttpUrlSubSeg(HttpUrlMatch sub = HOST) const
    {
        return m_smatch[sub];
    }
};

class HttpRequest
{
private:
    Buffer m_buffer;
    HttpUrl m_httpUrl;
    std::stringstream m_stream;
    int m_code;
    int m_sockfd;
    bool m_haveHandleHead;
    std::map<std::string, std::string> m_ackProperty;

private:
    void splitString(const std::string& s, std::vector<std::string>& v, const std::string& c);

public:
    enum HttpRequestMethod{
        GET = 0,
        POST,
    };

    HttpRequest(std::string HttpUrl);
    ~HttpRequest();

    void connect();
    void setRequestMethod(const std::string &method);
    void setRequestProperty(const std::string &key, const std::string &value);
    void setRequestBody(const std::string &content);

    void clearStream(){
        m_stream.str("");
    }

    std::string strStream() const
    {
        return m_stream.str();
    }

    int getResponseCode() const {
        assert(m_haveHandleHead);
        return m_code;
    }

    std::string getRequestProperty(const std::string &key) const{
        assert(m_haveHandleHead);
        return m_ackProperty.at(key);
    }

    std::string getResponseContent(){
        assert(m_haveHandleHead);
        return std::string(m_buffer.peek(), m_buffer.readableBytes());
    }

    void handleRead();
    void uploadFile(const std::string& file, const std::string& contentEnd);
    void downloadFile(const std::string& file);

    void send(){
        sockets::write(m_sockfd, strStream().c_str(), strStream().size());
    }

    void close(){
        sockets::close(m_sockfd);
    }
};




#endif