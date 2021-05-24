#include<cstdio>
#include<assert.h>
#include<algorithm>
#include"LogStream.hh"

LogStream::LogStream(){

}
LogStream::~LogStream(){

}

LogStream& LogStream::operator<<(bool v)
{
    m_buffer.append(v ? "1" : "0", 1);
    return *this;
}
LogStream& LogStream::operator<<(short v)
{
    *this << static_cast<int>(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned short v)
{
    *this << static_cast<unsigned int>(v);
    return *this;
}
LogStream& LogStream::operator<<(int v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned int v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(long v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned long v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(long long v) 
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(unsigned long long v)
{
    formatInteger(v);
    return *this;
}

const char digitsHex[] = "0123456789ABCDEF";

size_t convertHex(char buf[], uintptr_t value)
{
    uintptr_t i = value;
    char* p = buf;
    do
    {
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = digitsHex[lsd];
    } while (i != 0);

    *p = '\0';
    std::reverse(buf, p);
    return p-buf;    
}

LogStream& LogStream::operator<<(const void *p)
{
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if (m_buffer.avail() >= kMaxNumbericSize){
        
    }
}
LogStream& LogStream::operator<<(float v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(double)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(char v)
{
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(const char *)
{
    formatInteger(v);
    return *this;
}
