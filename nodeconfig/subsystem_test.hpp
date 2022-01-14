#ifndef SUBSYSTEM_HPP
#define SUBSYSTEM_HPP

#include "adas/node.hpp"
#include <chrono>
#include <iostream>
#include <algorithm>
#include <cmath>
#include "ehs3.hpp"
#include "eyeq4_object.hpp"

namespace subsystem_package {
    using namespace std::chrono_literals;
    using namespace adas::node;

    class subsystem_node1 {

    public:
        out<float> out1;
        out<float> out2;


    };

    class subsystem_node2 {

    public:
        in<float> in1;
        in<float> in2;
    };
}

#endif //SYBSYSTEM_HPP
