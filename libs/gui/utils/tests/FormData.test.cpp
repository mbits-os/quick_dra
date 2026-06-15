// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <QFile>
#include <app/utils/FormData.hpp>
#include <clocale>
#include <fstream>
#include <quick_dra/base/paths.hpp>
#include <quick_dra/version.hpp>
#include <sstream>
#include "FormDataPaths.hpp"
#include "TestApp.hpp"
#include "ui_helpers.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

#define NBSP "\xC2\xA0"

namespace quick_dra::gui::testing {
	namespace {
#ifdef WIN32
		struct win32_utf8 {
			win32_utf8() { SetConsoleOutputCP(CP_UTF8); }
		};

		win32_utf8 _utf8{};
#endif

		std::string file_contents(std::filesystem::path const& path) {
			auto file = std::ifstream{path};
			auto str = std::ostringstream{};
			str << file.rdbuf();
			return std::move(str).str();
		}

		std::string get_today(global_object const& state) {
			auto const var = state.typed_value(var::today, 1410y / July / 15);
			return fmt::format("{:04}-{:02}-{:02}", static_cast<int>(var.year()), static_cast<unsigned>(var.month()),
			                   static_cast<unsigned>(var.day()));
		}
	}  // namespace

	class FormData : public ::testing::Test {
	public:
		static void SetUpTestCase() { platform::config_data_dir(form_data_config); }
		static auto cliTestDataDir() { return std::filesystem::path{cli_test_data}; }

		static gui::FormData loadWithConfig(std::string_view filename) {
			gui::FormData data{};
			data.loadData();
			data.setConfig(cliTestDataDir() / filename, std::nullopt, false);
			return data;
		}
	};

	TEST_F(FormData, reportFormatIsAvailable) {
		auto file = QFile{":/report_format.yaml"};
		ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));
		ASSERT_TRUE(file.readAll().length());
	}

	TEST_F(FormData, load) {
		auto const data = loadWithConfig(".quick_dra.AB4123456_50671500000.quarter.yaml"sv);

		ASSERT_EQ(data.last_access, std::filesystem::last_write_time(data.cfg_path));
		ASSERT_EQ(data.templates.reports.size(), 2);
		ASSERT_EQ(data.gui_formats.size(), 2);
		ASSERT_TRUE(data.cfg.payer);
		ASSERT_TRUE(data.cfg.insured);
		ASSERT_EQ(data.cfg.insured->size(), 1);
		ASSERT_TRUE(data.tax_cfg);

		EXPECT_NE(data.templates.reports.find("DRA"s), data.templates.reports.end());
		EXPECT_NE(data.templates.reports.find("RCA"s), data.templates.reports.end());

		EXPECT_NE(data.gui_formats.find("DRA"s), data.gui_formats.end());
		EXPECT_NE(data.gui_formats.find("RCA"s), data.gui_formats.end());

		EXPECT_EQ(data.cfg.payer->first_name, "Jan"sv);
		EXPECT_EQ(data.cfg.payer->last_name, "Nowak"sv);
		EXPECT_EQ(data.cfg.insured->front().first_name, "Piotr"sv);
		EXPECT_EQ(data.cfg.insured->front().last_name, "Iksiński"sv);
	}

	TEST_F(FormData, ReportId) {
		ASSERT_EQ(ReportId{}.index, 1);
		ASSERT_EQ(ReportId{}.date.year(), 0y);
		ASSERT_EQ(ReportId{}.date.month(), January);
		ASSERT_FALSE(ReportId{}.isOverriden);
		ASSERT_NE(ReportId{}, ReportId{.date = 2024y / 8});
		ASSERT_NE(ReportId{.date = 2024y / 8}, (ReportId{.index = 2, .date = 2024y / 8}));
	}

	TEST_F(FormData, updateData) {
		std::setlocale(LC_ALL, "pl_PL.UTF-8");
		auto data = loadWithConfig(".quick_dra.AB4123456_50671500000.quarter.yaml"sv);
		data.lookupParameters({.date = 2024y / 5});

		EXPECT_EQ(data.tax_params.scale, std::map({std::pair{30000_PLN, 12_per}, std::pair{120000_PLN, 32_per}}));
		EXPECT_EQ(data.tax_params.minimal_pay, 4242_PLN);
		ASSERT_EQ(data.forms.size(), 2);
		EXPECT_EQ(data.forms[0].key, "RCA"sv);
		EXPECT_EQ(data.forms[1].key, "DRA"sv);
		EXPECT_EQ(data.summary.size(), 3);
		EXPECT_EQ(data.summary[0].index, 0);
		EXPECT_EQ(data.summary[0].label, "IKSIŃSKI, PIOTR"sv);
		EXPECT_EQ(data.summary[0].value, "915,11" NBSP "zł"sv);
		EXPECT_EQ(data.summary[0].comment,
		          "*społeczne:* 335,53" NBSP "zł, *zdrowotne:* 0,00" NBSP "zł, *podatek:* 0,00" NBSP "zł"sv);
		EXPECT_EQ(data.summary[1].index, 1);
		EXPECT_EQ(data.summary[1].label, "Dla ZUS"sv);
		EXPECT_EQ(data.summary[1].value, "335,53" NBSP "zł"sv);
		EXPECT_EQ(data.summary[1].comment, ""sv);
		EXPECT_EQ(data.summary[2].index, gui::FormData::InvalidIndex);
		EXPECT_EQ(data.summary[2].label, "Dla Urzędu Skarbowego"sv);
		EXPECT_EQ(data.summary[2].value, "0,00" NBSP "zł"sv);
		EXPECT_EQ(data.summary[2].comment, ""sv);
	}

	TEST_F(FormData, formatInvalid) {
		auto data = loadWithConfig(".quick_dra.AB4123456_50671500000.quarter.yaml"sv);
		data.lookupParameters({.date = 2024y / 5});
		auto const actualInvalidIndex = data.formatReport(gui::FormData::InvalidIndex);
		auto const actual2 = data.formatReport(2);
		EXPECT_EQ(actualInvalidIndex, actual2);
		EXPECT_EQ(actualInvalidIndex.title, "! <internal error>"sv);
		EXPECT_TRUE(actualInvalidIndex.data.empty());
		EXPECT_TRUE(actualInvalidIndex.order.empty());

		data.templates.reports.erase("RCA"s);
		auto const actualNoTemplate = data.formatReport(0);
		EXPECT_EQ(actualNoTemplate.title, "! RCA <internal error>"sv);
		EXPECT_TRUE(actualNoTemplate.data.empty());
		EXPECT_TRUE(actualNoTemplate.order.empty());
	}

	TEST_F(FormData, formatRCU) {
		std::setlocale(LC_ALL, "pl_PL.UTF-8");
		auto data = loadWithConfig(".quick_dra.AB4123456_50671500000.quarter.yaml"sv);
		data.lookupParameters({.date = 2024y / 5});
		auto const actual = data.formatReport(0);

		EXPECT_EQ(actual.title, "RCA (PIOTR IKSIŃSKI)"sv);
		EXPECT_EQ(actual.order, (std::vector{"I"s, "II"s, "III.A"s, "III.B"s, "III.C"s, "IV"s}));

		EXPECT_EQ(actual.data.at("I"s).name, "I"sv);
		EXPECT_EQ(actual.data.at("I"s).label, "DANE ORGANIZACYJNE"sv);

		EXPECT_EQ(actual.data.at("II"s).name, "II"sv);
		EXPECT_EQ(actual.data.at("II"s).label, "DANE IDENTYFIKACYJNE PŁATNIKA SKŁADEK"sv);

		EXPECT_EQ(actual.data.at("III.A"s).name, "III. A"sv);
		EXPECT_EQ(actual.data.at("III.A"s).label, "DANE IDENTYFIKACYJNE OSOBY UBEZPIECZONEJ"sv);

		EXPECT_EQ(actual.data.at("III.B"s).name, "III. B"sv);
		EXPECT_EQ(actual.data.at("III.B"s).label, "ZESTAWIENIE NALEŻNYCH SKŁADEK NA UBEZPIECZENIA SPOŁECZNE"sv);

		EXPECT_EQ(actual.data.at("III.C"s).name, "III. C"sv);
		EXPECT_EQ(actual.data.at("III.C"s).label, "ZESTAWIENIE NALEŻNYCH SKŁADEK NA UBEZPIECZENIE ZDROWOTNE"sv);

		EXPECT_EQ(actual.data.at("IV"s).name, "IV"sv);
		EXPECT_EQ(actual.data.at("IV"s).label, "OŚWIADCZENIE PŁATNIKA SKŁADEK"sv);
	}

	TEST_F(FormData, store) {
		auto data = loadWithConfig(".quick_dra.AB4123456_50671500000.quarter.yaml"sv);
		data.lookupParameters({.date = 2024y / 5});

		TmpDir tmp_dir{};  //-V821
		auto const cfg_path = tmp_dir.cwd() / ".quick_dra.yaml";
		auto const xml_path = tmp_dir.cwd() / "report.xml";

		ASSERT_FALSE(std::filesystem::exists(cfg_path));
		ASSERT_FALSE(std::filesystem::exists(xml_path));

		static constexpr unsigned short v2 = 2u;
		data.cfg.version = v2;
		data.cfg.payer->first_name = "First"sv;
		data.cfg.payer->last_name = "Last"sv;
		data.cfg_path = cfg_path;

		data.storeConfig();
		data.storeKedu(xml_path);

		ASSERT_TRUE(std::filesystem::is_regular_file(cfg_path));
		ASSERT_TRUE(std::filesystem::is_regular_file(xml_path));

		static constexpr auto expected_config =
		    R"($schema: 'https://raw.githubusercontent.com/mbits-os/quick_dra/refs/heads/main/data/schemas/user_config_schema.yaml'
wersja: 2
płatnik:
  nazwisko: 'Last, First'
  paszport: AB4123456
  nip: 7680002466
  pesel: 26211012346
ubezpieczeni:
  - nazwisko: 'Iksiński, Piotr'
    tytuł ubezpieczenia: 0110 0 0
    pesel: 50671500000
    historia:
      0/1:
        wymiar: 1/4
)"sv;

		auto const expected_xml = fmt::format(
		    "<KEDU wersja_schematu=\"1\" xmlns=\"http://www.zus.pl/2024/KEDU_5_6\">"
		    "<naglowek.KEDU>"
		    "<program><producent>midnightBITS</producent><symbol>Quick-DRA</symbol><wersja>{version}</wersja></program>"
		    "</naglowek.KEDU>"
		    // ZUSRCA
		    "<ZUSRCA id_dokumentu=\"1\">"
		    "<I><p1><p1>01</p1><p2>2024-05</p2></p1></I>"
		    "<II><p1>7680002466</p1><p3>26211012346</p3><p4>2</p4><p5>AB4123456</p5>"
		    "<p6>JAN NOWAK</p6><p7>NOWAK</p7><p8>JAN</p8><p9>2026-01-10</p9></II>"
		    "<III id_bloku=\"1\"><A><p1>IKSI\xC5\x83SKI</p1><p2>PIOTR</p2><p3>P</p3><p4>50671500000</p4></A>"
		    "<B><p1><p1>0110</p1><p2>0</p2><p3>0</p3></p1><p3><p1>1</p1><p2>4</p2></p3><p4>1060.50</p4><p5>1060.50</p5>"
		    "<p6>1060.50</p6><p7>103.50</p7><p8>15.91</p8><p9>25.98</p9><p10>0.00</p10><p11>103.50</p11>"
		    "<p12>68.93</p12><p13>0.00</p13><p14>17.71</p14><p29>335.53</p29></B>"
		    "<C><p1>665.00</p1><p4>0.00</p4></C>"
		    "<D><p1>0.00</p1><p2>0.00</p2><p3>0.00</p3><p4>0.00</p4></D></III>"
		    "<IV><p1>{today}</p1></IV>"
		    "</ZUSRCA>"
		    // ZUSDRA
		    "<ZUSDRA id_dokumentu=\"2\">"
		    "<I><p1>6</p1><p2><p1>01</p1><p2>2024-05</p2></p2></I>"
		    "<II><p1>7680002466</p1><p3>26211012346</p3><p4>2</p4><p5>AB4123456</p5><p6>JAN NOWAK</p6><p7>NOWAK</p7>"
		    "<p8>JAN</p8><p9>2026-01-10</p9></II>"
		    "<III><p1>1</p1><p3>1.67</p3></III>"
		    "<IV><p1>207.00</p1><p2>84.84</p2><p3>291.84</p3><p4>103.50</p4><p5>15.91</p5><p6>119.41</p6>"
		    "<p7>103.50</p7><p8>68.93</p8><p9>172.43</p9><p10>0.00</p10><p11>0.00</p11><p12>0.00</p12><p13>0.00</p13>"
		    "<p14>0.00</p14><p15>0.00</p15><p16>0.00</p16><p17>0.00</p17><p18>0.00</p18><p19>25.98</p19>"
		    "<p20>17.71</p20><p21>43.69</p21><p22>25.98</p22><p23>0.00</p23><p24>25.98</p24><p25>0.00</p25>"
		    "<p26>17.71</p26><p27>17.71</p27><p28>0.00</p28><p29>0.00</p29><p30>0.00</p30><p31>0.00</p31>"
		    "<p32>0.00</p32><p33>0.00</p33><p34>0.00</p34><p35>0.00</p35><p36>0.00</p36><p37>335.53</p37></IV>"
		    "<V><p1>0.00</p1><p2>0.00</p2><p3>0.00</p3><p4>0.00</p4><p5>0.00</p5></V>"
		    "<VI><p2>0.00</p2><p5>0.00</p5><p6>0.00</p6><p7>0.00</p7></VI>"
		    "<VII><p1>0.00</p1><p2>0.00</p2><p3>0.00</p3></VII>"
		    "<VIII><p3>0.00</p3></VIII>"
		    "<IX><p1>0.00</p1><p2>335.53</p2></IX>"
		    "<XIII><p1>{today}</p1></XIII>"
		    "</ZUSDRA>"
		    "</KEDU>",
		    fmt::arg("version", version::string), fmt::arg("today", get_today(data.forms.front().state)));

		EXPECT_EQ(file_contents(cfg_path), expected_config);
		EXPECT_EQ(file_contents(xml_path), expected_xml);
	}
}  // namespace quick_dra::gui::testing
