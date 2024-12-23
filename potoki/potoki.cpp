#include <iostream>
#include <ostream>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;

std::mutex coutMutex;

void print_hello(){
    coutMutex.lock();
    cout<<"hello from thread2!"<<endl;
    coutMutex.unlock();
}

int main(){
    cout<<"Main thread"<<endl;
    std::vector<std::thread> threads;
    for (int i=0; i<8; i++){
        threads.push_back(std::thread(print_hello));
    }
    cout<<"main thread"<<endl;
    for (auto& th:threads){
        if(th.joinable()){
            th.join();
        }

    }
}