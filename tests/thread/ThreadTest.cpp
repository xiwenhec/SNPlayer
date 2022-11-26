//
// Created by Sivin on 2022-11-26.
//
#include <iostream>
#include "utils/SNJson.h"
int main() {
    SNJson j;
    j["pi"] = 3.14;
    std::cout << j["pi"] << std::endl;
    std::cout << "thread test start\n";
    return 0;
}