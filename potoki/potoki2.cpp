#include <iostream>
#include <ostream>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;
std::mutex coutMutex;

void print_hello(){
    auto id=std::this_thread::get_id();

    coutMutex.lock();
    cout<<"thread "<<id<<endl;
    coutMutex.unlock();
}

int main(){
    std::vector<std::thread> threads;
    for (int i=0; i<8; i++){
        threads.push_back(std::thread(print_hello));
    }
    coutMutex.lock();
    cout<<"main thread"<<endl;
    coutMutex.unlock();
    for (auto& th:threads){
        if(th.joinable()){
            th.join();
        }

    }
}