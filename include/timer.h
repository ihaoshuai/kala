#pragma once

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstring>

struct Timer 
{
private:
    using Clock = std::chrono::steady_clock;
    char* buf_;
    static const size_t BUF_SIZE = 32;
    Clock::duration elapsed_;
    Clock::time_point start_;
    bool pause_;

public:
    Timer(double basic, bool pause)
        :start_(Clock::now()), pause_(pause)
    {
        elapsed_ = std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(basic));
        buf_ = new char[BUF_SIZE];
    }

    ~Timer() {
        delete [] buf_;
    }

    Timer(const Timer& other) : elapsed_(other.elapsed_), start_(other.start_), pause_(other.pause_) {
        buf_ = new char[BUF_SIZE];
        std::memcpy(buf_, other.buf_, BUF_SIZE);
    }
    Timer& operator=(const Timer& other) {
        if (this != &other) {
            std::memcpy(buf_, other.buf_, BUF_SIZE);   // 缓冲区大小固定，无需重新分配
            elapsed_ = other.elapsed_;
            start_ = other.start_;
            pause_ = other.pause_;
        }
        return *this;
    }


    void reset()
    {
        pause_ = true;
        elapsed_ = Clock::duration::zero();
        start_ = Clock::now();
    }

    void pause()
    {
        if(pause_) 
            return;

        elapsed_ += Clock::now() - start_;
        pause_ = true;
    }

    void resume()
    {
        if(!pause_)
            return;

        start_ = Clock::now();
        pause_ = false;
    }

    double elapsed() const {
        auto duration = elapsed_;
        if(!pause_)
            duration += Clock::now() - start_;
        return std::chrono::duration<double>(duration).count();
    }

    bool isPause() const {
        return pause_;
    }

    char* display() {
        //目标字符串不同单位个数, 如HH:MM:SS, 3
        const int need_time_num = 3;
        int divs[need_time_num] = { 24, 60, 60 };
        auto time = elapsed();
        int res[need_time_num] = { 0, 0, (int)time };
        for(int i=need_time_num-1; i>0; i--)
        {
            res[i-1] = res[i]/divs[i];
            if(res[i-1] == 0)
                break;
            res[i] = res[i]%divs[i];
        }
        snprintf(buf_, BUF_SIZE, "%02d:%02d:%02d", res[0], res[1], res[2]);
        return buf_;
    }

};

