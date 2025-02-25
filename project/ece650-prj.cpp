#include <iostream>
#include <vector>
#include <set>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <ctime>
#include <pthread.h>
#include <future>
#include "a4.hpp"

std::mutex mtx;
std::condition_variable cv;
std::atomic<int> gr1(0), gr2(0), gr3(0);
std::atomic<bool> eofReached(false);

std::vector<std::vector<int>> adjList;
std::set<std::pair<int, int>> edgeSet;
int num = 0;

void getThreadCpuTime(pthread_t thread, timespec &ts) {
    clockid_t cid;
    pthread_getcpuclockid(thread, &cid);
    clock_gettime(cid, &ts);
}

void consumer(int id) {
	pthread_t threadId = pthread_self();
	timespec start, end;
    std::string methodName = "";
	
    while (true) {
    	std::unique_lock<std::mutex> lock(mtx);
    	getThreadCpuTime(threadId, start);
        if (id == 1) {
            cv.wait(lock, [] { return gr1 == 1; });
            if (eofReached) {
            	break;
			}
            VertexCover(num, adjList);
            methodName = "CNF-SAT-VC ";
            gr1 = 0;
        } else if (id == 2) {
            cv.wait(lock, [] { return gr2 == 1; });
            if (eofReached) {
            	break;
			}
            ApproxVC1(num, adjList);
            methodName = "APPROX-VC-1 ";
            gr2 = 0;  
        } else if (id == 3) {
            cv.wait(lock, [] { return gr3 == 1; });
            if (eofReached) {
            	break;
			}
            ApproxVC2(edgeSet);
            methodName = "APPROX-VC-2 ";
            gr3 = 0;
        }
        getThreadCpuTime(threadId, end);
        cv.notify_all();
    }
}

void producer() {
	while (true) {
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait(lock, [] { return gr1 == 0 && gr2 == 0 && gr3 == 0; });
        if (std::cin.eof()) {
        	eofReached = true;
        	break;
		}
        std::string line;
        std::getline(std::cin, line);
        if (line.size() == 0) {
            continue;
        }
        
    	char cmd = line[0];
    	    if (cmd == 'V') {
    	    	//clear
        		adjList.clear();
        		edgeSet.clear();
    	    	VertexSpec(line, num, adjList);
			}
			else if (cmd == 'E') {
				EdgeSpec(line, num, adjList, edgeSet);
				gr1 = 1;
        		gr2 = 1;
       	 		gr3 = 1;
       	 		cv.notify_all();
			} 
	}
	gr1 = 1;
    gr2 = 1;
    gr3 = 1;
    cv.notify_all();
}

int main(int argc, char **argv)
{
	std::thread prod(producer);
    std::thread t1(consumer, 1);
    std::thread t2(consumer, 2);
    std::thread t3(consumer, 3);

    prod.join();
    t1.join();
    t2.join();
    t3.join();
    
    return 0;
}
