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
        
        void tick(std::string name, unsigned long long int start)
        {
            tickTimings[name] = start;
            timings[name]=0;
        }
        
        void tock(std::string name, unsigned long long int end)
        {
            float duration = (float)(end - tickTimings[name]) / 1000.0f;
            
            if(duration > 0)
            {
                timings[name] = duration;
                updateStates[name] = true;
            }
        }

        std::map<std::string, double> &getTimings(){return timings;}

        std::map<std::string, std::atomic_bool> &getUpdateStats(){return updateStates;}

        bool updated (std::string name) {return updateStates[name];}
    private:
        std::map<std::string, double> timings;
        std::map<std::string, std::atomic_bool> updateStates;
        std::map<std::string, unsigned long long int> tickTimings;
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
        
    public:
        Watch watch;
        std::map<std::string, std::vector<std::string> > log;
    };
//}

#endif /* LogUtil_hpp */
