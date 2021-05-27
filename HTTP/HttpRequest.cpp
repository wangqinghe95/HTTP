#include "HttpRequest.hh"

#include <endian.h>
#include <sys/uio.h>    //readv
#include <unistd.h> //read
#include <string>
#include <fstream>

const std::map<std::string, int>::value_type init_value[] = 
{
    std::map<std::string, int>::value_type("GET", HttpRequest::GET),
    std::map<std::string, int>::value_type("POST", HttpRequest::POST),
};

const static std::map<std::string, int> kRequestMethodMap(init_value, init_value 
                                    + (sizeof init_value / sizeof init_value[0]));

const inline uint16_t hostToNetwork16(uint16_t host16)
{
    return htobe16(host16);
}

int sockets::createSocket(sa_family_t family)
{
    int sock;

#ifdef SOCK_CLOEXEC
    sock = socket(family, SOCK_STREAM | SOCK_CLOEXEC , 0);
    if (-1 != sock || errno != EINVAL ){
        return sock;
    }
#endif

    sock = socket(family, SOCK_STREAM, 0);

#ifdef FD_CLOEXEC
    if (-1 != sock){
        fcntl(sock, F_SETFD, FD_CLOEXEC);
    }
#endif

    return sock;
}

int sockets::connect(int sockfd, const struct sockaddr* addr)
{
    return ::connect(sockfd, addr, sizeof(struct sockaddr));
}

void sockets::fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        std::cout << "sockets::fromIpPort" << std::endl;
    }
}

size_t sockets::read(int sockfd, void *buf, size_t count)
{
    return ::read(sockfd, buf, count);
}
size_t sockets::readv(int sockfd, const struct iovec* iov, int iovcnt)
{
    return ::readv(sockfd, iov, iovcnt);
}
size_t sockets::write(int sockfd, const void *buf, size_t count)
{
    return ::write(sockfd, buf, count);
}

void sockets::close(int sockfd)
{
    if (::close(sockfd) < 0){
        std::cout << "sockets::close" << std::endl;
    }
}

void sockets::delaySecond(int sec)
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = 0;
    select(0, NULL, NULL, NULL, &tv);
}

InetAddress::InetAddress(std::string ip, uint16_t port)
{
    ::bzero(&m_addr, sizeof(m_addr));
    sockets::fromIpPort(ip.c_str(), port, &m_addr);
}

InetAddress::~InetAddress()
{
    
}

uint32_t InetAddress::ipNetEndian() const
{
    assert(family() == AF_INET);
    return m_addr.sin_addr.s_addr;
}

HttpRequest::HttpRequest(std::string httpUrl)
  :m_httpUrl(httpUrl)
{

}

HttpRequest::~HttpRequest()
{

}

void HttpRequest::connect()
{
    char ip[32] = {0};
    while (true)
    {
        struct hostent* phost = NULL;

        phost = gethostbyname(m_httpUrl.domain().c_str());
        if (NULL == phost){
            std::cout <<  "HttpUrlToIp(): gethostbyname error : " 
                        << errno << " : "<< strerror(errno) << " continue." << std::endl;
            sockets::delaySecond(1);
            continue;
        }

        inet_ntop(phost->h_addrtype, phost->h_addr, ip, sizeof(ip));
        std::cout << "HttpRequest::Connector() gethostbyname Successful" << std::endl;

        InetAddress serverAddr = InetAddress(ip, 80);
        m_sockfd = sockets::createSocket(serverAddr.family());
        if (0 > m_sockfd){
            std::cout << "HttpRequest::connect() : createSocket error" << std::endl;
        }

        int ret = sockets::connect(m_sockfd, serverAddr.getSockAddr());
        std::cout << "sockfd : " << m_sockfd << "sockets::connect ret : " << ret << std::endl;

        int savedErrno = (ret == 0) ? 0 : errno;

        switch (savedErrno)
        {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            std::cout << "HttpRequest::connect() sockfd : " << m_sockfd << " Successful" << std::endl;
            break;
        
        default:
            std::cout << "Connect error" << std::endl;
            sockets::delaySecond(1);
            continue;
        }

        break;  //while
    }

    std::cout << "HttpRequest::Connector() end" << std::endl;    
}

void HttpRequest::setRequestMethod(const std::string &method)
{
    switch (kRequestMethodMap.at(method))
    {
    case HttpRequest::GET :
        m_stream << "GET /" << m_httpUrl.getHttpUrlSubSeg(HttpUrl::URI) << " HTTP/1.1\r\n";
        std::cout << m_stream.str().c_str();
        break;
    case HttpRequest::POST :
        m_stream << "POST /" << m_httpUrl.getHttpUrlSubSeg(HttpUrl::URI) << " HTTP/1.1\r\n";
        std::cout << m_stream.str().c_str();
        break;
    default:
        std::cout << "No such Method : " << method.c_str();
        break;
    }

    m_stream << "Host: " << m_httpUrl.getHttpUrlSubSeg(HttpUrl::HOST) << "\r\n";
}

void HttpRequest::setRequestProperty(const std::string &key, const std::string &value)
{
    m_stream << key << ": " << value << "\r\n";
}
void HttpRequest::setRequestBody(const std::string &content)
{
    m_stream << content;
}

void HttpRequest::handleRead()
{
    assert(!m_haveHandleHead);
    ssize_t nread = 0;
    ssize_t writtenBytes = 0;

    nread = sockets::read(m_sockfd, m_buffer.beginWrite(), kBufferSize);
    if (0 > nread){
        std::cout << "sockets::read" << std::endl;
    }

    m_buffer.hasWritten(nread);

    std::cout << "sockets::read(): nread: " << nread << " remain: " << m_buffer.wirteableBytes() << std::endl;

    size_t remain = kBufferSize - nread;
    while (remain > 0)
    {
        size_t n = sockets::read(m_sockfd, m_buffer.beginWrite(), remain);
        if (0 > n){
            std::cout << "sockets::read" << std::endl;
        }

        m_buffer.hasWritten(n);
        if (0 == n){
            std::cout << "sockets::read finish" << std::endl;
            break;
        }

        remain -= n;
    }
    
    int headsize = 0;
    std::string line;
    std::stringstream ss(m_buffer.peek());

    std::vector<std::string> v;
    getline(ss, line);

    headsize += line.size() + 1;
    splitString(line, v, " ");
    m_code = std::stoi(v[1]);
    if (v[1] != "200"){
        std::cout << "Error Http Server Response : " << v[1].c_str() << std::endl;
    }

    do
    {
        getline(ss, line);
        headsize += line.size() + 1;
        if (!line.empty()){
            line.erase(line.end() - 1);
            v.clear();
            splitString(line, v, ":");
            if (v.size() == 2){
                m_ackProperty[v[0]] = v[1].erase(0, v[1].find_first_not_of(" "));
            }
        }
    } while (!line.empty());

    std::cout << "Http Head Size is " << headsize << std::endl;
    
    std::string res(m_buffer.peek(), headsize);
    std::cout << "Http Response :\n" << res << std::endl;
    m_buffer.retrieve(headsize);
    m_haveHandleHead = true;
}

void HttpRequest::uploadFile(const std::string& file, const std::string& contentEnd)
{
    FILE* fp = fopen(file.c_str(), "rb");
    if (fp == NULL){
        std::cout << "fopen() File :" << file.c_str() << " Errno" << std::endl;
    }

    bool isEnd = false;
    ssize_t writtenBytes = 0;

    assert(m_buffer.wirteableBytes() == Buffer::kInitialSize);

    while (!isEnd)
    {
        ssize_t nread = fread(m_buffer.beginWrite(), 1, kBufferSize, fp);
        m_buffer.hasWritten(nread);
        while (m_buffer.wirteableBytes() > 0)
        {
            std::cout << "file read(): nread: " << nread << " remain: " 
                        << m_buffer.wirteableBytes() << std::endl;
            size_t n = fread(m_buffer.beginWrite(), 1, m_buffer.wirteableBytes(), fp);
            m_buffer.hasWritten(n);
            if (0 == n){
                int err = ferror(fp);
                if (err){
                    fprintf(stderr, "fread failed: %s\n", strerror(err));
                }
                std::cout << "sockets::read finish";
                isEnd = true;
                break;
            }
        }

        ssize_t nwrite = sockets::write(m_sockfd, m_buffer.peek(), m_buffer.readableBytes());
        if (0 > nwrite){
            std::cout << "sockets::write" << std::endl;
        }

        writtenBytes += nwrite;
        std::cout << "sockets::write nread " << m_buffer.readableBytes() 
                << " nwrite " << nwrite << " writtenBytes " << writtenBytes << std::endl;
        m_buffer.retrieve(nwrite);
        
    }

    fclose(fp);

    m_buffer.retriveAll();

    ssize_t n = sockets::write(m_sockfd, contentEnd.c_str(), contentEnd.size());
    if (0 > n){
        std::cout << "sockets::write" << std::endl;
    }
    
}
void HttpRequest::downloadFile(const std::string& file)
{
    assert(m_haveHandleHead);
    bool isEnd = false;
    ssize_t nread = 0;
    ssize_t writtenBytes = 0;
    bool haveHandleHead = false;
    bool isDownFile = false;

    std::ofstream output(file, std::ios::binary);
    if (!output){
        std::cout << "open file error" << file << std::endl;
    }

    output.write(m_buffer.peek(), m_buffer.readableBytes());
    writtenBytes += m_buffer.readableBytes();
    m_buffer.retrieve(m_buffer.readableBytes());

    std::cout << "Content-Length: " << getRequestProperty("Content-Length");

    while (!isEnd)
    {
        nread = sockets::read(m_sockfd, m_buffer.beginWrite(), kBufferSize);
        if (0 < nread){
            std::cout << "sockets::read" << std::endl;
        }

        m_buffer.hasWritten(nread);
        std::cout << "sockets::read(): nread: " << nread << " remain: " 
                << m_buffer.wirteableBytes() << " writtenBytes: " << writtenBytes << std::endl;

        size_t remain = kBufferSize - nread;
        while (remain > 0)
        {
            size_t n = sockets::read(m_sockfd, m_buffer.beginWrite(), remain);
            if (n < 0){
                std::cout << "sockets::read failed" << std::endl; 
            }

            m_buffer.hasWritten(nread);
            if (0 == nread){
                std::cout << "sockets::read finish" << std::endl;
                isEnd = true;
                break;
            }
            remain = remain - n;
        }

        output.write(m_buffer.peek(), m_buffer.readableBytes());
        writtenBytes += m_buffer.readableBytes();
        m_buffer.retrieve(m_buffer.readableBytes());
        
    }

    std::cout << " writtenBytes " << writtenBytes << std::endl;

    output.close();
    sockets::close(m_sockfd);
}

void HttpRequest::splitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while (std::string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2-pos1));
        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }

    if (pos1 != s.length()){
        v.push_back(s.substr(pos1));
    }    
}






