#include "gpiopin.hpp"

void GPIOPin::set(const bool &val) {
	GPIO_WriteBit(port, pin, static_cast<BitAction>(val));
}
bool GPIOPin::get(void) const {
	return GPIO_ReadInputDataBit(port, pin);
}
GPIOPin& GPIOPin::operator=(const bool &val) {
	set(val);
	return *this;
}
GPIOPin::operator bool() const {
	return get();
}
