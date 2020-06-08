#include <gtest/gtest.h>

#include <boost/format.hpp>
#include <iostream>
#include <rxcpp/rx.hpp>

std::mutex lock;

void log(const std::string &msg) {
    std::unique_lock<std::mutex> l_lock(lock);
    std::cout << msg << std::endl;
}

TEST(test, test_1) {
    rxcpp::subjects::subject<int> subject;
    log((boost::format("main %1%") % std::this_thread::get_id()).str());
    auto cs1 = subject.get_observable()
                   .map([](int i) {
                       log((boost::format("cs1:map: %1% %2%") %
                            std::this_thread::get_id() % i)
                               .str());
                       return i + i;
                   })
                   .observe_on(rxcpp::observe_on_new_thread())
                   .subscribe([](int i) {
                       log((boost::format("cs1: %1% %2%") %
                            std::this_thread::get_id() % i)
                               .str());
                   });

    auto cs2 = subject.get_observable()
                   .map([](int i) {
                       log((boost::format("cs2:map: %1% %2%") %
                            std::this_thread::get_id() % i)
                               .str());
                       return i * i;
                   })
                   .observe_on(rxcpp::observe_on_new_thread())
                   .subscribe([](int i) {
                       log((boost::format("cs2: %1% %2%") %
                            std::this_thread::get_id() % i)
                               .str());
                   });

    subject.get_subscriber().on_next(1);
    subject.get_subscriber().on_next(2);

    cs1.unsubscribe();

    subject.get_subscriber().on_next(3);
    subject.get_subscriber().on_next(4);

    cs2.unsubscribe();

    subject.get_subscriber().on_next(5);

    std::this_thread::sleep_for(std::chrono::seconds(5));
}
