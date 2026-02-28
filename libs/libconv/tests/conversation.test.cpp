// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <fmt/std.h>
#include <gtest/gtest.h>
#include <concepts>
#include <quick_dra/base/str.hpp>
#include <quick_dra/base/types.hpp>
#include <quick_dra/conv/conversation.hpp>
#include <quick_dra/conv/validators.hpp>
#include <sstream>

// This macro is for implementing ASSERT_DEATH*, EXPECT_DEATH*,
// ASSERT_EXIT*, and EXPECT_EXIT*.
#define GTEST_NO_DEATH_TEST_(statement, predicate, regex_or_matcher, fail)    \
	GTEST_AMBIGUOUS_ELSE_BLOCKER_                                             \
	if (::testing::internal::AlwaysTrue()) {                                  \
		::testing::internal::DeathTest* gtest_dt;                             \
		if (!::testing::internal::DeathTest::Create(                          \
		        #statement,                                                   \
		        ::testing::internal::MakeDeathTestMatcher(regex_or_matcher),  \
		        __FILE__, __LINE__, &gtest_dt)) {                             \
			goto GTEST_CONCAT_TOKEN_(gtest_label_, __LINE__);                 \
		}                                                                     \
		if (gtest_dt != nullptr) {                                            \
			std::unique_ptr< ::testing::internal::DeathTest> gtest_dt_ptr(    \
			    gtest_dt);                                                    \
			switch (gtest_dt->AssumeRole()) {                                 \
				case ::testing::internal::DeathTest::OVERSEE_TEST:            \
					if (gtest_dt->Passed(predicate(gtest_dt->Wait()))) {      \
						goto GTEST_CONCAT_TOKEN_(gtest_label_, __LINE__);     \
					}                                                         \
					break;                                                    \
				case ::testing::internal::DeathTest::EXECUTE_TEST: {          \
					const ::testing::internal::DeathTest::ReturnSentinel      \
					    gtest_sentinel(gtest_dt);                             \
					GTEST_EXECUTE_DEATH_TEST_STATEMENT_(statement, gtest_dt); \
					gtest_dt->Abort(::testing::internal::DeathTest::          \
					                    TEST_ENCOUNTERED_RETURN_STATEMENT);   \
					break;                                                    \
				}                                                             \
			}                                                                 \
		}                                                                     \
	} else                                                                    \
		GTEST_CONCAT_TOKEN_(gtest_label_, __LINE__)                           \
		    : fail(::testing::internal::DeathTest::LastMessage())

#define EXPECT_NO_EXIT(statement, predicate, regex) \
	GTEST_NO_DEATH_TEST_(statement, predicate, regex, GTEST_NONFATAL_FAILURE_)

#define EXPECT_NO_DEATH(statement, regex) \
	EXPECT_NO_EXIT(statement, ::testing::internal::ExitedUnsuccessfully, regex)

namespace quick_dra::testing {
	using std::literals::operator""s;
	using std::literals::operator""sv;

	struct conversation : quick_dra::conversation<partial::insured_t> {};

	struct simple_capture_stdout {
		simple_capture_stdout(std::string* tgt) : tgt{tgt} {
			::testing::internal::CaptureStdout();
		}
		~simple_capture_stdout() {
			*tgt = ::testing::internal::GetCapturedStdout();
		}

	private:
		std::string* tgt;
	};

	struct capture_stdout : simple_capture_stdout {
		capture_stdout(std::string* tgt,
		               conversation& conv,
		               std::string const& in = {})
		    : simple_capture_stdout{tgt}, Stdin{in} {
			conv.attach(Stdin);
		}

	private:
		std::istringstream Stdin;
	};

	struct capture_stderr {
		capture_stderr(std::string* tgt, std::string const& in = {})
		    : tgt{tgt}, Stdin{in} {
			::testing::internal::CaptureStderr();
		}
		~capture_stderr() { *tgt = ::testing::internal::GetCapturedStderr(); }

	private:
		std::string* tgt;

	public:
		std::istringstream Stdin;
	};

	TEST(conversation, check_field) {
		conversation conv{};
		std::string Stdout{};
		bool result{false};

		{
			capture_stdout capture{&Stdout, conv};
			result = conv.check_field(builtin::policies::passport);
		}
		EXPECT_FALSE(result);
		EXPECT_EQ(Stdout, "\033[0;36mPassport\033[m> "sv);

		{
			capture_stdout capture{&Stdout, conv, "EH0123456\n"s};
			result = conv.check_field(builtin::policies::passport);
		}
		EXPECT_TRUE(result);
		EXPECT_EQ(Stdout, "\033[0;36mPassport\033[m> "sv);
		EXPECT_EQ(conv.dst.passport, "EH0123456"sv);

		conv.dst.passport.reset();
		conv.ask_questions = false;
		{
			capture_stdout capture{&Stdout, conv};
			result = conv.check_field(builtin::policies::passport);
		}
		EXPECT_TRUE(result);
		EXPECT_EQ(Stdout, ""sv);
		EXPECT_FALSE(conv.dst.passport);

		conv.opts.passport = "EH0123456"s;
		conv.dst.passport.reset();
		{
			capture_stdout capture{&Stdout, conv};
			result = conv.check_field(builtin::policies::passport);
		}
		EXPECT_TRUE(result);
		EXPECT_EQ(Stdout, ""sv);
		EXPECT_EQ(conv.dst.passport, "EH0123456"sv);

		conv.opts.passport = "EH1123456"s;
		conv.dst.passport.reset();
		{
			capture_stdout capture{&Stdout, conv};
			result = conv.check_field(builtin::policies::passport);
		}
		EXPECT_FALSE(result);
		EXPECT_EQ(Stdout,
		          "\033[0;90m - The passport number provided seems to be "
		          "invalid.\033[m\n"
		          "\033[0;90m - Cannot save invalid data with -y. "
		          "Stopping.\033[m\n"sv);
		EXPECT_EQ(conv.dst.passport, "EH1123456"sv);
	}

	TEST(conversation, check_enum_field) {
		conversation conv{};
		std::string Stdout{};
		bool result{false};

		{
			capture_stdout capture{&Stdout, conv};
			result = conv.check_enum_field(
			    ""sv, builtin::policies::kind, builtin::policies::document,
			    get_enum_item(builtin::policies::social_id),
			    get_enum_item(builtin::policies::id_card),
			    get_enum_item(builtin::policies::passport));
		}
		EXPECT_FALSE(result);
		EXPECT_EQ(
		    Stdout,
		    "\033[0;36mDocument kind\033[0;90m [P - PESEL, 1 - ID card, 2 - Passport]\033[m> "sv);

		conv.dst.kind = "2"s;
		conv.dst.document = "AA0000000"s;
		conv.opts.social_id = "26211012346"s;
		{
			capture_stdout capture{&Stdout, conv, "\n\n"s};
			result = conv.check_enum_field(
			    ""sv, builtin::policies::kind, builtin::policies::document,
			    get_enum_item(builtin::policies::social_id),
			    get_enum_item(builtin::policies::id_card),
			    get_enum_item(builtin::policies::passport));
		}
		EXPECT_TRUE(result);
		EXPECT_EQ(Stdout,
		          "\033[0;36mDocument kind\033[0;90m [[P] - PESEL, 1 - ID "
		          "card, 2 - Passport]\033[m> "
		          "\033[0;36mPESEL\033[0;90m [26211012346]\033[m> "sv);
		EXPECT_EQ(conv.dst.kind, "P"s);
		EXPECT_EQ(conv.dst.document, "26211012346"s);

		conv = {};
		{
			capture_stdout capture{&Stdout, conv, "P\nINVALID\n"s};
			result = conv.check_enum_field(
			    ""sv, builtin::policies::kind,
			    // std::cin, ""sv, builtin::policies::kind,
			    builtin::policies::document,
			    get_enum_item(builtin::policies::social_id),
			    get_enum_item(builtin::policies::id_card),
			    get_enum_item(builtin::policies::passport));
		}
		EXPECT_FALSE(result);
		EXPECT_EQ(
		    Stdout,
		    "\033[0;36mDocument kind\033[0;90m [P - PESEL, 1 - ID card, 2 - "
		    "Passport]\033[m> "
		    "\033[0;36mPESEL\033[m> "
		    "\033[0;90m - The social id provided seems to be invalid.\033[m\n"
		    "\033[0;36mPESEL\033[m> "sv);
		EXPECT_FALSE(conv.dst.kind);
		EXPECT_FALSE(conv.dst.document);

		conv = {};
		conv.ask_questions = false;
		{
			capture_stdout capture{&Stdout, conv};
			result = conv.check_enum_field(
			    ""sv, builtin::policies::kind, builtin::policies::document,
			    get_enum_item(builtin::policies::social_id),
			    get_enum_item(builtin::policies::id_card),
			    get_enum_item(builtin::policies::passport));
		}
		EXPECT_FALSE(result);
		EXPECT_EQ(Stdout, ""sv);

		{
			capture_stdout capture{&Stdout, conv};
			result = conv.check_enum_field(
			    "one of <those args>"sv, builtin::policies::kind,
			    builtin::policies::document,
			    get_enum_item(builtin::policies::social_id),
			    get_enum_item(builtin::policies::id_card),
			    get_enum_item(builtin::policies::passport));
		}
		EXPECT_FALSE(result);
		EXPECT_EQ(
		    Stdout,
		    "\033[0;90m - Cannot guess document kind based on information given. Please use either one of <those args> with -y\033[m\n"sv);
	}

	template <FieldPolicyWithArgFlags... Policy>
	void check_required(conversation& conv, Policy const&... policies) {
		arg_parser p{"tool"sv, {}, ""sv};

		conv.ask_questions = false;
		conv.verifier(p.parser).required(policies...);
	}

	TEST(conversation, verify_standalone) {
		conversation conv{};
		EXPECT_DEATH(check_required(conv, builtin::policies::first_name.through(
		                                      "--name"sv)),
		             "tool: error: argument --name is required with -y");

		conv.dst.first_name = "Name present on dst"s;
		EXPECT_NO_DEATH(
		    check_required(conv,
		                   builtin::policies::first_name.through("--name"sv)),
		    "error");

		conv.dst.first_name.reset();
		conv.opts.first_name = "Name present on opts"s;
		EXPECT_NO_DEATH(
		    check_required(conv,
		                   builtin::policies::first_name.through("--name"sv)),
		    "");
	}

	void check_first_second_and_third(conversation& conv) {
		check_required(conv,  //
		               builtin::policies::first_name.through("--first"sv),
		               builtin::policies::last_name.through("--second"sv),
		               builtin::policies::social_id.through("--third"sv));
	}

	void clean_conv_(conversation& conv,
	                 FieldPolicy<std::string> auto const&... cleaners) {
		(cleaners.select(conv.dst).reset(), ...);
		(cleaners.select(conv.opts).reset(), ...);
	};

	void clean_conv(conversation& conv) {
		clean_conv_(conv, builtin::policies::first_name,
		            builtin::policies::last_name, builtin::policies::social_id);
	}

	void set_conv_and_check(conversation& conv,
	                        FieldPolicy<std::string> auto const&... policy) {
		clean_conv(conv);

		((policy.select(conv.opts) = "value"s), ...);
		check_first_second_and_third(conv);
	};

	void expect_no_death_after_setting(
	    conversation& conv,
	    FieldPolicy<std::string> auto const& policy) {
		clean_conv(conv);

		policy.select(conv.dst) = "value"s;
		policy.select(conv.opts).reset();
		EXPECT_NO_DEATH(check_first_second_and_third(conv), "");
		policy.select(conv.dst).reset();
		policy.select(conv.opts) = "value"s;
		EXPECT_NO_DEATH(check_first_second_and_third(conv), "");
	};

	TEST(conversation, verify_exclusive_extrema) {
		conversation conv{};
		EXPECT_DEATH(check_first_second_and_third(conv),
		             "tool: error: at least one of --first, --second and "
		             "--third is required with -y");

		expect_no_death_after_setting(conv, builtin::policies::first_name);
		expect_no_death_after_setting(conv, builtin::policies::last_name);
		expect_no_death_after_setting(conv, builtin::policies::social_id);
	}

	TEST(conversation, verify_broken_exclusive) {
		conversation conv{};
		EXPECT_DEATH(check_first_second_and_third(conv),
		             "tool: error: at least one of --first, --second and "
		             "--third is required with -y");

		EXPECT_DEATH(set_conv_and_check(conv, builtin::policies::first_name,
		                                builtin::policies::last_name,
		                                builtin::policies::social_id),
		             "tool: error: only one of --first, --second and --third "
		             "can be used at the same time");

		EXPECT_DEATH(set_conv_and_check(conv, builtin::policies::first_name,
		                                builtin::policies::last_name),
		             "tool: error: only one of --first and --second can be "
		             "used at the same time");

		EXPECT_DEATH(set_conv_and_check(conv, builtin::policies::first_name,
		                                builtin::policies::social_id),
		             "tool: error: only one of --first and --third can be used "
		             "at the same time");
	}

	struct arg_stg {
		::args::args_view argv() noexcept {
			return ::args::from_main(static_cast<int>(stg_.size()),
			                         argv_.data());
		}

		template <std::same_as<std::string_view>... Args>
		arg_stg(Args... arg) : stg_{std::string{arg.data(), arg.size()}...} {}

	private:
		std::vector<std::string> stg_{};
		std::vector<char*> argv_{to_C()};

		std::vector<char*> to_C() {
			std::vector<char*> args{};
			args.resize(stg_.size() + 1);

			std::transform(stg_.begin(), stg_.end(), args.begin(),
			               [](auto& s) { return s.data(); });
			args[stg_.size()] = nullptr;

			return args;
		}
	};

	template <std::same_as<std::string_view>... Args>
	std::string test_arguments(partial::insured_t& opts, Args... args) {
		auto tested_args = arg_stg{"tool"sv, args...};
		::args::null_translator tr{};
		::args::parser parser{""s, tested_args.argv(), &tr};

		parser.arg(opts.title, "title")
		    .meta("<code>")
		    .help(
		        "select insurance title code as six digits in `#### # #' "
		        "format; for instance, for title of 0110, no social benefits, "
		        "no disability, it should be \"0110 0 0\"");
		parser.arg(opts.part_time_scale, "scale")
		    .meta("<num>/<den>")
		    .help(
		        "for part time workers, what scale should be applied to their "
		        "salary; defaults to 1/1");
		parser.arg(opts.salary, "salary")
		    .meta("<zł>")
		    .help(
		        "select gross salary amount, before applying the scale, "
		        "represented by a number with 0.01 increment, with optional "
		        "PLN or zł suffix; alternatively, single word \"minimal\" to "
		        "represent a minimal pay in a given month");

		std::string Stdout{};
		{
			simple_capture_stdout capture{&Stdout};
			parser.parse();
		}
		return Stdout;
	}

	TEST(conversation, args_convert) {
		partial::insured_t opts{};
		EXPECT_NO_DEATH(test_arguments(opts), "");
		EXPECT_NO_DEATH(
		    test_arguments(opts, "--title"sv, "9999 8 7"sv, "--scale"sv,
		                   "3/4"sv, "--salary"sv, "3000zł"sv),
		    "");
		test_arguments(opts, "--title"sv, "9999 8 7"sv, "--scale"sv, "3/4"sv,
		               "--salary"sv, "minimal"sv);
		EXPECT_EQ(opts.title, (insurance_title{
		                          .title_code{"9999"s},
		                          .pension_right{8},
		                          .disability_level{7},
		                      }));
		EXPECT_EQ(opts.part_time_scale, (ratio{3, 4}));
		EXPECT_EQ(opts.salary, minimal_salary);

		test_arguments(opts, "--salary"sv, "3000zł"sv);
		EXPECT_EQ(opts.salary, 3000_PLN);

		test_arguments(opts, "--salary"sv, "none"sv);
		EXPECT_EQ(opts.salary, minimal_salary);

		EXPECT_DEATH(
		    test_arguments(opts, "--title"sv, "bad"sv),
		    "tool: error: --title: expecting 6 digits in form `#### # #'");

		EXPECT_DEATH(test_arguments(opts, "--scale"sv, "bad"sv),
		             "tool: error: --scale: expecting two numbers in form "
		             "`<num>/<den>`, with denominator not equal to zero");

		EXPECT_DEATH(test_arguments(opts, "--salary"sv, "bad"sv),
		             "tool: error: --salary: expecting a number with 0.01 "
		             "increment, with optional PLN or zł suffix");
	}

	TEST(conversation, show_modified) {
		partial::insured_t orig{};
		orig.first_name = "James"s;
		orig.last_name = "Kirk"s;
		orig.kind = "P"s;
		orig.document = "26211012346"s;
		orig.title = insurance_title{"0110"s, 0, 0};
		orig.part_time_scale = ratio{3, 4};
		orig.salary = minimal_salary;

		conversation conv{};
		conv.dst = orig;

		conv.dst.first_name = "Tiberius"s;
		conv.dst.kind = "2"s;
		conv.dst.document = "EH0123456"s;
		conv.dst.salary = 10'000_PLN;

		std::string Stdout{};
		{
			capture_stdout capture{&Stdout, conv};
			conv.show_modified(builtin::policies::first_name, orig);
			conv.show_modified(builtin::policies::last_name, orig);
			conv.show_modified(builtin::policies::kind, orig);
			conv.show_modified(builtin::policies::document, orig);
			conv.show_modified(builtin::policies::title, orig);
			conv.show_modified(builtin::policies::part_time_scale, orig);
			conv.show_modified(builtin::policies::salary, orig);
		}

		EXPECT_EQ(
		    Stdout,
		    "\033[0;90mFirst name changed from \033[mJames\033[0;90m to "
		    "\033[mTiberius\n\033[0;90mDocument kind changed from "
		    "\033[mP\033[0;90m to \033[m2\n\033[0;90mDocument changed from "
		    "\033[m26211012346\033[0;90m to \033[mEH0123456\n\033[0;90mSalary "
		    "changed from \033[mminimal for a given month\033[0;90m to "
		    "\033[m10000 zł\n"
		    ""sv);
	}

	TEST(conversation, show_added) {
		conversation conv{};
		conv.dst.first_name = "James"s;
		conv.dst.last_name = "Kirk"s;
		conv.dst.part_time_scale = ratio{3, 4};

		std::string Stdout{};
		{
			capture_stdout capture{&Stdout, conv};
			conv.show_added(builtin::policies::first_name);
			conv.show_added(builtin::policies::last_name);
			conv.show_added(builtin::policies::kind);
			conv.show_added(builtin::policies::document);
			conv.show_added(builtin::policies::title);
			conv.show_added(builtin::policies::part_time_scale);
			conv.show_added(builtin::policies::salary);
		}

		EXPECT_EQ(Stdout,
		          "\x1B[0;90mFirst name set to \x1B[mJames\n"
		          "\x1B[0;90mLast name set to \x1B[mKirk\n"
		          "\x1B[0;90mDocument kind set to \x1B[m<empty>\n"
		          "\x1B[0;90mDocument set to \x1B[m<empty>\n"
		          "\x1B[0;90mInsurance title set to \x1B[m<empty>\n"
		          "\x1B[0;90mPart-time scale set to \x1B[m3/4\n"
		          "\x1B[0;90mSalary set to \x1B[m<empty>\n"sv);
	}
}  // namespace quick_dra::testing
