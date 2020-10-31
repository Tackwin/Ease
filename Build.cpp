#include "src/BuildSelf.hpp"
#include "src/BuildSelf.cpp"

using namespace BuildSelf;

Build build(Flags flags) noexcept {
	Build build = Build::get_default(flags);

	build.name = "BS";
	build.target = Build::Target::Header_Only;

	build.add_source("./src/BuildSelf.cpp");
	build.add_header("./src/BuildSelf.hpp");

	return build;
}