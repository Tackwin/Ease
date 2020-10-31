#include "src/Ease.hpp"
#include "src/Ease.cpp"

Build build(Flags flags) noexcept {
	Build build = Build::get_default(flags);

	build.name = "Ease";
	build.target = Build::Target::Header_Only;
	build.invert_header_implementation_define = true;

	build.add_source("./src/Ease.cpp");
	build.add_header("./src/Ease.hpp");

	return build;
}