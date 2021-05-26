#ifndef _CONDITION_H_
#define _CONDITION_H_

#include <condition_variable>
#include <mutex>
#include <chrono>

class Condition
{
private:
    Condition(const Condition&);
    const Condition& operator=(const Condition&);

    std::condition_variable m_cond;
public:
    explicit Condition(){}
    ~Condition();

    void wait(std::unique_lock<std::mutex>& lock){
        m_cond.wait(lock);
    }

    void waitForSeconds(std::unique_lock<std::mutex>& lock, double seconds){
        const int64_t kNanoSecondsPerSecond = 1000000000;
        int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);

        m_cond.wait_for(lock, std::chrono::nanoseconds(nanoseconds));
    }

    void notify(){
        m_cond.notify_one();
    }

    void notifyAll(){
        m_cond.notify_all();
    }
};



#endif