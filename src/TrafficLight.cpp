#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <queue>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // COMPLETE! FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this] { return !_messages.empty(); });

    T msg = std::move(_messages.back());
    _messages.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // COMPLETE! FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    std::lock_guard<std::mutex> lock(_mutex);
    _messages.push_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // COMPLETE! FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        TrafficLightPhase phase = _queue.receive();
        if (phase == TrafficLightPhase::green) return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // COMPLETE! FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 

    // launch cycleThroughPhases function in a thread
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}


// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // COMPLETE! FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    // create monitor object as a shared pointer
    //std::shared_ptr<MessageQueue<TrafficLightPhase>> queue(new MessageQueue<TrafficLightPhase>);
    
    // store current time as start time
    auto startTime = std::chrono::system_clock::now();
    
    // generate random cycle duration between 4 and 6 seconds
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<double> distr(4.0, 6.0);
    double cycleDuration = distr(eng);

    while (true)
    {
        auto currentTime = std::chrono::system_clock::now();
        auto totalSeconds = std::chrono::duration<double>(currentTime - startTime);

        if (totalSeconds.count() >= cycleDuration)
        {
            // toggle current phase
            _currentPhase = _currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red;
            
            // update start time for next cycle
            startTime = std::chrono::system_clock::now();

            // send a message to the queue
            //std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send, queue, std::move(_currentPhase));
            TrafficLightPhase phaseMsg = _currentPhase;
            _queue.send(std::move(phaseMsg));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
