#include <concepts>
#include <iostream>

template <typename T>
concept Number = std::is_arithmetic_v<T>;

template <typename T>
concept Printable = requires(T t) {
  std::cout << t;
};

template <typename T>
concept Powerable = requires(T t, bool on) {
  { t.power(on) } -> std::convertible_to<void>;
} || requires(T t) {
  { t.turnOn() } -> std::convertible_to<void>;
  { t.turnOff() } -> std::convertible_to<void>;
};

class PowerableObject1 {
public:
    void power(bool on) {
        if (on) {
            std::cout << "Powering on..." << std::endl;
        } else {
            std::cout << "Powering off..." << std::endl;
        }
    }
};

// Sample class with turnOn and turnOff functions
class PowerableObject2 {
public:
    void turnOn() {
        std::cout << "Turning on..." << std::endl;
    }

    void turnOff() {
        std::cout << "Turning off..." << std::endl;
    }
};

template <Powerable T>
void powerOnOff(T obj, bool on) {
    if constexpr (std::is_invocable_v<decltype(&T::power), T, bool>) {
        obj.power(on);
    } else {
        if (on) {
            obj.turnOn();
        } else {
            obj.turnOff();
        }
    }
}

int main() {
    PowerableObject1 obj1;
    powerOnOff(obj1, true);
    powerOnOff(obj1, false);

    PowerableObject2 obj2;
    //powerOnOff(obj2, true);
    //powerOnOff(obj2, false);

    return 0;
}