#include <spsc.hpp>
#include <thread>
#include <iostream>

int main()
{
    auto &registry = QueueRegistry<int>::instance();
    auto queue = registry.get_queue("my_queue");

    // 生产者线程
    std::thread producer([&queue]()
                         {
        for (int i = 0; i < 10; ++i)
        {
            queue->push(i);
            std::cout<<"Produced: " << i << std::endl;
        } });

    // 消费者线程
    std::thread consumer([&queue]()
                         {
        for (int i = 0; i < 10; ++i)
        {
            auto value = queue->pop();
            std::cout<<"Consumed: " << value << std::endl;
            // 处理数据
        } });

    producer.join();
    consumer.join();

    return 0;
}
