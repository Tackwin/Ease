#include "Ease.hpp"

Build build(Flags flags) noexcept {
	auto b = Build::get_default(flags);
	b.add_source("Main.cpp");
	b.add_source("Extern.cpp");
	return b;
}