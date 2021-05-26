#ifndef _TIME_STAMP_HH
#define _TIME_STAMP_HH

#include<cstdint>
#include<string>

class TimeStamp{
public:
    TimeStamp():m_mincroSecondSinceEpoch(0){}

    explicit TimeStamp(int64_t m_mincroSecondSinceEpochArg):
        m_mincroSecondSinceEpoch(m_mincroSecondSinceEpochArg){}
    
    int64_t mincroSecondSinceEpoch() const{
        return m_mincroSecondSinceEpoch;
    }

    std::string toString() const;

    bool valid() const{
        return m_mincroSecondSinceEpoch > 0;
    }

    static TimeStamp now();
    static TimeStamp invalid(){
        return TimeStamp();
    }

    static const int kMicroSecondPerSecond = 1000 * 1000;

private:
    int64_t m_mincroSecondSinceEpoch;
};

namespace times{
    inline TimeStamp addTime(TimeStamp timestamp, double seconds)
    {
        int64_t delta = static_cast<int64_t>(seconds* TimeStamp::kMicroSecondPerSecond);
        return TimeStamp(timestamp.mincroSecondSinceEpoch() + delta);
    }
}

inline bool operator< (TimeStamp lhs, TimeStamp rhs)
{
    return lhs.mincroSecondSinceEpoch() < rhs.mincroSecondSinceEpoch();
}

inline bool operator==(TimeStamp lhs, TimeStamp rhs)
{
    return lhs.mincroSecondSinceEpoch() == rhs.mincroSecondSinceEpoch();
}

#endif