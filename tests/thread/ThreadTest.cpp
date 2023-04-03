//
// Created by Sivin on 2022-11-26.
//

#include "utils/SNTimer.h"
#include <cstdio>
#include <memory>
#define LOG_TAG "ThreadTest"

#include <thread>
#include <iostream>
#include "utils/SNLog.h"
#include "utils/SNThread.h"

using namespace Sivin;

void test1() {
  int i = 0;
  std::thread t1{[&i]() {
    while (true) {
      printf("thread 1 i= %d\n", i++);
    }
  }};
  std::thread t2{[&i]() {
    while (true) {
      printf("thread 2 i= %d\n", i++);
    }
  }};

  SNTimer::sleepMs(100000000);
  t1.join();
  t2.join();
}

void test2(int loop) {

  int i = 0;
  auto *thread1 = new SNThread([&i, loop] {
    printf("test %d thread 1 i= %d\n", loop, i++);
    // SNTimer::sleepMs(30);
    return 0;
  },
      "test1Thread");

  auto *thread2 = new SNThread([&i, loop] {
    printf("test %d thread 2 i= %d\n", loop, i++);
    // SNTimer::sleepMs(30);
    return 0;
  },
      "test2Thread");

  thread1->start();
  thread2->start();
  SNTimer::sleepMs(1000);

  thread1->pause();

  SNTimer::sleepMs(1000);

  thread2->pause();
  thread1->start();

  SNTimer::sleepMs(1000);
  thread1->stop();
  thread2->stop();
}

int main() {
  for (int i = 0; i < 100; i++) {
    printf("---------------------test :%d start-----\n", i);
    test2(i);
    printf("----------------------test :%d end-------\n", i);
    printf("\n");
    printf("\n");
  }

  std::cout << "test end " << std::endl;
  return 0;
}