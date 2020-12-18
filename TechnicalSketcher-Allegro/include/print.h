#pragma once

template<typename T>
void print(T msg) {
	std::cout << msg << std::endl;
}

template<typename T, typename... Trecurse>
void print(T msg, Trecurse... recurse) {
	std::cout << msg;
	print(recurse...);
}
