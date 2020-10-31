#include "src/BuildSelf.hpp"
#include "src/BuildSelf.cpp"

Build build(Flags flags) noexcept {
	Build build = Build::get_default(flags);

	build.name = "BS";
	build.target = Build::Target::Header_Only;
	build.invert_header_implementation_define = true;

	build.add_source("./src/BuildSelf.cpp");
	build.add_header("./src/BuildSelf.hpp");

	return build;
}