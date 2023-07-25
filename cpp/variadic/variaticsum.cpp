#include <iostream>

template <typename ... Args>
void sum(Args ... args) {
    // Calculate the sum using a fold expression
    auto total = (args + ...);
    
    // Output the result
    std::cout << "Sum: " << total << std::endl;
}

int main() {
    sum(1, 2, 3, 4, 5); // Output: Sum: 15
    sum(10, 20, 30);    // Output: Sum: 60
    sum(7, 13, 25);     // Output: Sum: 45
    
    return 0;
}