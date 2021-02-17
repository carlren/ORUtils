//
//  LogUtil.hpp
//  DFGGUI
//
//  Created by Shun-Cheng Wu on 06/Jan/2019.
//

#ifndef LogUtil_hpp
#define LogUtil_hpp

#include <stdio.h>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <atomic>
#include <iostream>
#include<sys/time.h>


//////
/// DEBUG MESSAGE
//////
#ifdef NDEBUG
#define DEBUG(x, ...)
#else
//#define DEBUG(x) do { std::cerr << x << std::endl; } while (0)
#define DEBUG(x, ...) do { \
fprintf(stderr, x,  ##__VA_ARGS__); } while (0)
#endif
#ifdef NDEBUG
#define DEBUG_MSG(str) do { } while ( false )
#else
#define DEBUG_MSG(str) do { std::cout << str; } while( false )
#endif

//////
/// RELEASE MESSAGE
//////
#ifdef NRMSG
#define RPRINTF(x, ...)
#else
#define RPRINTF(x, ...) do { \
fprintf(stderr, x,  ##__VA_ARGS__); } while (0)
#endif
#ifdef NRMSG
#define RCOUT(str) do { } while ( false )
#else
#define RCOUT(str) do { std::cout << str; } while( false )
#endif



//////
/// TIMER
//////
#define getWatch Monitoring::getInstance().watch
#define getMonitor Monitoring::getInstance()


#ifndef DISABLE_STOPWATCH
    // 開始計時
#define TICK(name) \
do \
{ \
getWatch.tick(name, getWatch.getCurrentSystemTime()); \
} \
while(false)
    // 停止計時
#define TOCK(name) \
do {\
getWatch.tock(name, getWatch.getCurrentSystemTime()); \
} while(false)

    // 停止且輸出
#define TOCK_P(name) \
do {\
TOCK(name); \
printf("[Time] %s: %f\n", name, getWatch.getTimings()[name]); \
} while(false)
/**
 * CTICK: start without reset time
 * CTOCK_ACC: accumulate times
 */
#define CTICK_RESET(name) \
do{\
getWatch.reset(name); \
} while (false)

#define CTICK(name) \
do {\
getWatch.start(name,getWatch.getCurrentSystemTime()); \
} while (false)

#define CTOCK_ACC(name) \
do {\
getWatch.tock_accumulate(name,getWatch.getCurrentSystemTime()); \
} while (false)

#define CTOCK(name) \
do {\
getWatch.clip(name,getWatch.getCurrentSystemTime()); \
} while (false)

#define TOCK_IP(name, value) do{\
TOCK(name);\
    double time = getWatch.getTimings()[name]; \
    if(time > value) printf("[Time] %s: %f\n", name, getWatch.getTimings()[name]); \
} while (false)

// 開始＋停止
#define STOPWATCH(name, expression) \
do { \
TICK(name); \
expression; \
TOCK(name); \
} while(false)

#define STOPWATCH_P(name, expression) \
do {\
TICK(name); \
expression; \
TOCK_P(name); \
} while(false)



    
#else

#define STOPWATCH(name, expression) expression
#define STOPWATCH_P(name, expression) expression
    
#define TOCK(name) ((void)0)
    
#define TICK(name) ((void)0)
#define TOCK_P(name) ((void)0)
    
#endif
    
    class Watch {
    public:
        void addStopwatchTiming(std::string name, unsigned long long int duration)
        {
            if(duration > 0)
            {
                timings[name] = (float)(duration) / 1000.0f;
            }
        }
        
        static unsigned long long int getCurrentSystemTime()
        {
            timeval tv;
            gettimeofday(&tv, 0);
            unsigned long long int time = (unsigned long long int)(tv.tv_sec * 1000000 + tv.tv_usec);
            return time;
        }

        inline void tick(const std::string &name, unsigned long long int start)
        {
            tickTimings[name] = start;
            timings[name]=0;
        }

        inline void tock(const std::string &name, unsigned long long int end)
        {
            float duration = (float)(end - tickTimings[name]) / 1000.0f;
            
            if(duration > 0)
            {
                timings[name] = duration;
                updateStates[name] = true;
            }
        }

        void reset(const std::string &name){
            mClipTimes[name] = {0.f,0};
        }
        inline void start(const std::string &name, unsigned long long int start){;
            tickTimings[name] = start;
        }
        void clip(const std::string &name, unsigned long long int end){
            float duration = (float)(end - tickTimings[name]) / 1000.0f;
            if(duration > 0)
            {
                if(mClipTimes.find(name) == mClipTimes.end())
                    mClipTimes[name] = {0,0};
                mClipTimes[name].first += duration;
                mClipTimes[name].second += 1;
                updateStates[name] = true;
            }
        }

        inline void tock_accumulate(const std::string &name, unsigned long long int end) {
            float duration = (float)(end - tickTimings[name]) / 1000.0f;
            if(duration > 0)
            {
                if(timings.find(name) == timings.end())
                    timings[name]=0;
                timings[name] += duration;
                updateStates[name] = true;
            }
        }

        std::map<std::string, double> &getTimings(){return timings;}
        std::map<std::string, std::pair<float,int>> &getCTimings(){return mClipTimes;}
        std::map<std::string, std::atomic_bool> &getUpdateStats(){return updateStates;}

        bool updated (std::string name) {return updateStates[name];}
    private:
        std::map<std::string, double> timings;
        std::map<std::string, std::atomic_bool> updateStates;
        std::map<std::string, unsigned long long int> tickTimings;
        std::map<std::string, std::pair<float,int>> mClipTimes;
    };


    template<typename IndexT, typename ValueT>
    struct LogEntry {
        IndexT x;
        ValueT y;
        LogEntry()=default;
        LogEntry(IndexT x_, ValueT y_):x(std::move(x_)),y(std::move(y_)){}
    };
    
    // Singleton config class
    class Monitoring
    {
    public:
        // get instanse
        static Monitoring& getInstance()
        {
            static Monitoring ConfigGUIInstance;
            return ConfigGUIInstance;
        }
        Monitoring(){};

        void Log(const std::string &name, unsigned int stamp, const std::string &value) {
            if(log.find(name) == log.end())
                log[name].reserve(2<<16); // reserve 65536
            log[name].emplace_back(stamp,value);
        }

    public:
        Watch watch;
        std::map<std::string, std::vector<LogEntry<unsigned int, std::string>> > log;
    };
//}

#endif /* LogUtil_hpp */
