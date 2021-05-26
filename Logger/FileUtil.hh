#ifndef _FILE_UTIL_HH
#define _FILE_UTIL_HH

#include<string>
class StringArg
{
private:
    const char* m_str;
public:
    StringArg(const char* str):m_str(str){}
    StringArg(const std::string& str):m_str(str.c_str()){}
    const char* c_str(){
        return m_str;
    }
    ~StringArg();
};

namespace FileUtil{

    class AppendFile
    {
    private:
        static const size_t KFileBufferSize = 4096;
        size_t write(const char* logline, int len);

        FILE* m_fp;
        off_t m_writtenBytes;
        char m_buffer[KFileBufferSize];
    public:

        explicit AppendFile(StringArg filePath);
        ~AppendFile();

        void append(const char* logline, const size_t len); //调用系统函数 fwrite 函数写入文件
        size_t write(const char* logline, const size_t len);

        void flush();   //刷新缓存

        off_t writtenBytes() const{ //获取已写字节数
            return m_writtenBytes;
        }
    };
        
}


#endif