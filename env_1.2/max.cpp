#include <iostream>

template <typename T>
void print(T val) {
	std::cout << val << '\n';
}

template <typename T>
T max(T arg1, T arg2) {
	if(arg1 < arg2){
	return arg2;
	}
	return arg1;
}



int main(int argc, char* argv[]) {
if(argc != 3) {
	std::cerr << "invalid argument count\n";
	return 1;
}
	auto first = atoi(argv[1]);
	auto  second = atoi(argv[2]);

	print(max(first, second));
	return 0;
}
