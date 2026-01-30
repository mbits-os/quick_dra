// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <ryml.hpp>
#include <ryml_std.hpp>
#include <string_view>

namespace yaml {
	inline std::string_view view(ryml::csubstr const& sub) {
		if (sub.empty()) return {};
		return {sub.data(), sub.size()};
	}

	struct c4_error_exception {};

	struct ref_ctx;

	struct base_ctx {
		struct error_handler {
			error_handler();
			~error_handler();

			void install_in_c4();

			bool handle_msg(c4::yml::Location const& loc,
			                std::string_view msg,
			                std::string_view level);
			bool handle_error(c4::yml::Location const& loc,
			                  std::string_view msg);

			bool ok() const noexcept { return parse_succeeded; }
			bool failed() const noexcept { return !parse_succeeded; }

		private:
			bool parse_succeeded = true;
			error_handler* prev = nullptr;
		};

		ryml::Parser const* parser{nullptr};

		ref_ctx from(ryml::ConstNodeRef const& ref) const;
	};

	struct ref_ctx : base_ctx {
		ryml::ConstNodeRef const* ref_{};

		bool error(std::string_view const& msg) const;
		void warn(std::string_view const& msg) const;
		ryml::ConstNodeRef const& ref() const noexcept { return *ref_; }
		c4::csubstr val() const { return ref_ ? ref_->val() : c4::csubstr{}; }
	};
}  // namespace yaml
