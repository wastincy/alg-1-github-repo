#include <iostream>
#include <ostream>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;
std::mutex coutMutex;

void print_hello(int id, double& sum){
    
    coutMutex.lock();
    sum += id;
    cout<<"thread "<<id<<": "<<sum<<endl;
    coutMutex.unlock();
}

int main(){
    double sum = 0;
    std::vector<std::thread> threads;
    for (int i=0; i<8; i++){
        threads.push_back(std::thread(print_hello, i, std::ref(sum)));
    }
    coutMutex.lock();
    cout<<"main thread"<<endl;
    coutMutex.unlock();
    for (auto& th:threads){
        if(th.joinable()){
            th.join();
        }

    }
    cout<<"sum= "<<sum<<endl;
}