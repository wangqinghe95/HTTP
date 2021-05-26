#include<sys/time.h>
#include<inttypes.h>
#include"TimeStamp.hh"

TimeStamp TimeStamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    int64_t seconds = tv.tv_sec;
    return TimeStamp(seconds*kMicroSecondPerSecond + tv.tv_usec);
}

std::string TimeStamp::toString() const
{
    char buf[32] = {0};
    int64_t seconds = m_mincroSecondSinceEpoch / kMicroSecondPerSecond;
    int64_t microseconds = m_mincroSecondSinceEpoch % kMicroSecondPerSecond;
    snprintf(buf, sizeof(buf) - 1, "%" PRId64 "%.%06" PRId64 "", seconds, microseconds);
    return buf;
}
