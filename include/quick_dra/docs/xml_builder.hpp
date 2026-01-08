// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <quick_dra/docs/forms.hpp>
#include <quick_dra/model/model.hpp>

namespace quick_dra {
	struct xml;

	xml build_kedu_doc(std::string_view program_name, std::string_view version);

	void attach_document(xml& root,
	                     bool verbose,
	                     form const& form,
	                     std::vector<compiled_section> const& tmplt,
	                     unsigned doc_id);
	void store_xml(xml const& tree, std::string const& filename);
}  // namespace quick_dra
