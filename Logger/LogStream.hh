#ifndef _LOG_STREAM_HH
#define _LOG_STREAM_HH

#include<cstdio>
#include<cstring>
#include<string>

const int kSmallBuffer = 4096;
const int kLargeBuffder = 4096;

template<int SIZE>
class LogBuffer
{
public:
    LogBuffer():m_cur(m_data){

    }
    ~LogBuffer();

    void append(const char* buf, size_t len){
        if ( (avail()) > len){  // m_data 的长度大于 buf，即传入的数据 m_cur 有足够的内存保存
            memcpy(m_cur, buf, len);
            m_cur += len;
        }
    }

    char* current(){
        return m_cur;
    }

    void add(size_t len){
        m_cur += len;
    }

    int length() const{
        return m_cur - m_data;
    }

    int avail() const{
        return static_cast<int>(end() - m_cur);
    }

    const char* data() const{
        return m_data;
    }

    void reset(){
        m_cur = m_data;
    }

    void bzero(){
        ::bzero(m_data, sizeof(m_data));
    }

private:
    char m_data[SIZE];
    char* m_cur;    //指向的是当前字符串尾部保存的位置，其中字符串是保存在 m_data 中

    const char* end const{
        return m_data + sizeof(m_data);
    }
};

class LogStream{
public:
    typedef LogBuffer<kSmallBuffer> Buffer;
    typedef LogStream self;
    
    LogStream();
    ~LogStream();

    self& operator<<(bool v);
    self& operator<<(short);
    self& operator<<(unsigned short);
    self& operator<<(int);
    self& operator<<(unsigned int);
    self& operator<<(long);
    self& operator<<(unsigned long);
    self& operator<<(long long);
    self& operator<<(unsigned long long);
    self& operator<<(const void *);
    self& operator<<(float v);
    self& operator<<(double);
    self& operator<<(char v);
    self& operator<<(const char *);
    self& operator<<(const std::string& s);

    void append(const char* data, int len){
        return m_buffer.append(data, len);
    }
    const Buffer& buffer() const{
        return m_buffer;
    }

private:
    Buffer m_buffer;
    static const int kMaxNumbericSize = 32; //静态常数据成员

    LogStream(const LogStream& ls); //no copyable
    LogStream& operator=(const LogStream& ls);
    template<typename T>
    void formatInteger(T v);
};

class Fmt{
public:
    template<typename T>
    Fmt(const char* fmt, T val);

    const char* data() const{
        return m_buf;
    }

    int length() const{
        return m_length;
    }

private:
    char m_buf[32];
    int m_length;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt){
    s.append(fmt.data(), fmt.length());
    return s;
}

#endif