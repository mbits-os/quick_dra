// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <yaml/ref.hpp>

using namespace std::literals;

namespace yaml {
	namespace {
		thread_local struct base_ctx::error_handler* head = nullptr;

		[[noreturn]] void c4_error_handler(const char* msg,
		                                   size_t msg_len,
		                                   c4::yml::Location loc,
		                                   void* user_data) {
			reinterpret_cast<base_ctx::error_handler*>(user_data)->handle_error(
			    loc, std::string_view{msg, msg_len});
			throw c4_error_exception{};
		}  // GCOV_EXCL_LINE
	}  // namespace

	base_ctx::error_handler::error_handler() : prev{head} { head = this; }
	base_ctx::error_handler::~error_handler() {
		head = prev;
		if (previous) {
			c4::yml::set_callbacks(*previous);
		}
	}

	void base_ctx::error_handler::install_in_c4() {
		previous = c4::yml::get_callbacks();
		c4::yml::Callbacks callbacks;
		callbacks.m_user_data = this;
		callbacks.m_error = c4_error_handler;
		c4::yml::set_callbacks(callbacks);
	}

	bool base_ctx::error_handler::handle_msg(c4::yml::Location const& loc,
	                                         std::string_view msg,
	                                         std::string_view level) {
		if (!loc.name.empty()) {
			fmt::print(stderr, "{}:", view(loc.name));
		}
		fmt::print(stderr, "{}:{}: {}: {}\n", loc.line + 1, loc.col + 1, level,
		           msg);

		auto stack = this;
		while (stack) {
			stack->parse_succeeded = false;
			stack = stack->prev;
		}

		return false;
	}

	bool base_ctx::error_handler::handle_error(c4::yml::Location const& loc,
	                                           std::string_view msg) {
		return handle_msg(loc, msg, "error"sv);
	}

	ref_ctx base_ctx::from(ryml::ConstNodeRef const& ref) const {
		return {
		    {.parser = parser},
		    &ref,
		};
	}

	bool ref_ctx::error(std::string_view const& msg) const {
		if (parser && parser->source().len && ref_ && head) {
			return head->handle_error(ref_->location(*parser), msg);
		}  // GCOV_EXCL_LINE
		[[unlikely]];                            // GCOV_EXCL_LINE
		fmt::print(stderr, "error: {}\n", msg);  // GCOV_EXCL_LINE
		return false;                            // GCOV_EXCL_LINE
	}
}  // namespace yaml
