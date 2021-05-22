#include "Ease.hpp"

/*
clang++ -o ./Build.exe .\Build.cpp  -std=c++17
*/

Build hello(Flags flags) noexcept {
	auto b = Build::get_default(flags);
	b.name = "Hello";
	b.add_source_recursively("hello/");
	return b;
}

Build extern_(Flags flags) noexcept {
	auto b = Build::get_default(flags);
	b.name = "Extern";
	b.add_source_recursively("extern/");
	return b;
}

Build main(Flags flags) noexcept {
	return Build::sequentials({
		hello(flags), extern_(flags)
	});
}



