//
// Created by Jing SHEN on 7/3/20.
//

#include "../include/fact.hpp"
unsigned int factorial( unsigned int number ) {
    return number <= 1 ? number : factorial(number-1)*number;
}