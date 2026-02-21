// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <quick_dra/docs/file_set.hpp>
#include <quick_dra/docs/forms.hpp>
#include <quick_dra/docs/xml_builder.hpp>
#include <quick_dra/version.hpp>
#include <vector>

namespace quick_dra {
	xml build_file_set(options const& opt,
	                   std::vector<quick_dra::form> const& forms,
	                   compiled_templates const& templates) {
		auto doc_id = 0u;
		auto root = build_kedu_doc(version::program, version::string);

		if (opt.verbose_level == verbose::templates) {
			templates.debug_print();
		}

		if (opt.verbose_level == verbose::calculated_sections) {
			fmt::print("-- filled forms:\n");
		}

		for (auto const& form : forms) {
			auto it = templates.reports.find(form.key);
			if (it == templates.reports.end()) continue;
			attach_document(root, opt.verbose_level, form, it->second,
			                ++doc_id);
		}

		return root;
	}  // GCOV_EXCL_LINE[GCC]
}  // namespace quick_dra
