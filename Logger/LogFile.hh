#ifndef _LOG_FILE_HH
#define _LOG_FILE_HH



#include<string>
#include<mutex>
#include"scoped_ptr.hh"

namespace FileUtil{
    class AppendFile;
}

/*
LogFile 日志管理类，完成日志文件的管理工作

*/

class LogFile
{
private:
    
    void append_unlocked(const char* logline, int len);
    bool rollFile();    // 滚动文件，当日志超过 m_roolSize 大小时会滚动一个新的日志文件出来

    const std::string m_filePath;
    const int m_flushInterval;

    int m_rollCnt;
    off_t m_roolSize;
    scoped_ptr<std::mutex> m_mutex; // 用于 append 数据时，给文件上锁
    scoped_ptr<FileUtil::AppendFile> m_file;

public:
    LogFile(const std::string& filePath
            , off_t rollSize = 2048*1000
            , bool threadSafe=true
            , int flushInterval = 0);

    ~LogFile();

    void append(const char* logline, int len);  // 写入日志
    void flush();   // 刷新缓冲

    // 用于滚动日志，给日志文件取名，以滚动时间作为后缀
    std::string getlogFileName(const std::string& baseName);
};


#endif