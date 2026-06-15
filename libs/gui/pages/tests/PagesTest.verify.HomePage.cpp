// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "page_verify.hpp"

#include <app/pages/HomePage.hpp>
#include <app/pages/PersonelPage.hpp>
#include <app/pages/ReportFormPage.hpp>
#include <app/pages/ReportIdEditPage.hpp>
#include <quick_dra/cli/builtins.hpp>
#include <quick_dra/version.hpp>

#define QUICK_DRA_VERSION QUICK_DRA_VERSION_STR QUICK_DRA_VERSION_STABILITY QUICK_DRA_VERSION_BUILD_META

using namespace quick_dra::gui;
using namespace quick_dra;

void verify_page(HomePage& page) {
	auto const group = page.reportButtons();
	auto const first = group->count() ? group->itemAt(0) : nullptr;
	QVERIFY(first && first->isClickable());

	QVERIFY_NAVIGATION(page.editReportIdAction(), ReportIdEditPage, "Identyfikator");
	QVERIFY_NAVIGATION(page.showPersonelFilesAction(), PersonelPage, "Dane osobowe");
	QVERIFY_NAVIGATION(first->clicked(), ReportFormPage, u"RCA (PIOTR IKSI\u0143SKI)");

	auto const today = get_today();
	auto const today_s = std::format("{:04}-{:02}-{:02}", static_cast<int>(today.year()),
	                                 static_cast<unsigned>(today.month()), static_cast<unsigned>(today.day()));
	{
		page.setProperty("kedu_path", QString{u"./here.xml"});
		page.storeKeduXmlLocally();
		auto const text =
		    QString{
		        uR"(<KEDU wersja_schematu="1" xmlns="http://www.zus.pl/2024/KEDU_5_6">)"
		        uR"(<naglowek.KEDU>)"
		        uR"(<program>)"
		        uR"(<producent>midnightBITS</producent>)"
		        uR"(<symbol>Quick-DRA</symbol>)"
		        uR"(<wersja>)" QUICK_DRA_VERSION R"(</wersja>)"
		        uR"(</program>)"
		        uR"(</naglowek.KEDU>)"
		        uR"(<ZUSRCA id_dokumentu="1">)"
		        uR"(<I><p1><p1>01</p1><p2>2020-05</p2></p1></I>)"
		        uR"(<II><p1>7680002466</p1><p3>26211012346</p3><p4>1</p4><p5>ABC523456</p5><p6>JAN NOWAK</p6><p7>NOWAK</p7><p8>JAN</p8><p9>2026-01-10</p9></II>)"
		        uR"(<III id_bloku="1">)"
		        uR"(<A><p1>IKSIŃSKI</p1><p2>PIOTR</p2><p3>1</p3><p4>ABC523456</p4></A>)"
		        uR"(<B><p1><p1>0110</p1><p2>0</p2><p3>0</p3></p1><p3><p1>1</p1><p2>4</p2></p3><p4>0.00</p4><p5>0.00</p5><p6>0.00</p6><p7>0.00</p7><p8>0.00</p8><p9>0.00</p9><p10>0.00</p10><p11>0.00</p11><p12>0.00</p12><p13>0.00</p13><p14>0.00</p14><p29>0.00</p29></B>)"
		        uR"(<C><p1>0.00</p1><p4>0.00</p4></C>)"
		        uR"(<D><p1>0.00</p1><p2>0.00</p2><p3>0.00</p3><p4>0.00</p4></D>)"
		        uR"(</III)"
		        uR"(><IV><p1>%1</p1></IV>)"
		        uR"(</ZUSRCA>)"
		        uR"(<ZUSRCA id_dokumentu="2">)"
		        uR"(<I><p1><p1>01</p1><p2>2020-05</p2></p1></I>)"
		        uR"(<II><p1>7680002466</p1><p3>26211012346</p3><p4>1</p4><p5>ABC523456</p5><p6>JAN NOWAK</p6><p7>NOWAK</p7><p8>JAN</p8><p9>2026-01-10</p9></II>)"
		        uR"(<III id_bloku="1">)"
		        uR"(<A><p1>KOWALSKI</p1><p2>ANTONI</p2><p3>P</p3><p4>78070707132</p4></A>)"
		        uR"(<B><p1><p1>0110</p1><p2>0</p2><p3>0</p3></p1><p3><p1>1</p1><p2>1</p2></p3><p4>0.00</p4><p5>0.00</p5><p6>0.00</p6><p7>0.00</p7><p8>0.00</p8><p9>0.00</p9><p10>0.00</p10><p11>0.00</p11><p12>0.00</p12><p13>0.00</p13><p14>0.00</p14><p29>0.00</p29></B>)"
		        uR"(<C><p1>0.00</p1><p4>0.00</p4></C>)"
		        uR"(<D><p1>0.00</p1><p2>0.00</p2><p3>0.00</p3><p4>0.00</p4></D>)"
		        uR"(</III>)"
		        uR"(<IV><p1>%1</p1></IV>)"
		        uR"(</ZUSRCA>)"
		        uR"(<ZUSRCA id_dokumentu="3">)"
		        uR"(<I><p1><p1>01</p1><p2>2020-05</p2></p1></I>)"
		        uR"(<II><p1>7680002466</p1><p3>26211012346</p3><p4>1</p4><p5>ABC523456</p5><p6>JAN NOWAK</p6><p7>NOWAK</p7><p8>JAN</p8><p9>2026-01-10</p9></II>)"
		        uR"(<III id_bloku="1">)"
		        uR"(<A><p1>IKSIŃSKA</p1><p2>MARIA</p2><p3>P</p3><p4>26211012346</p4></A>)"
		        uR"(<B><p1><p1>0110</p1><p2>0</p2><p3>0</p3></p1><p3><p1>1</p1><p2>1</p2></p3><p4>7500.00</p4><p5>7500.00</p5><p6>7500.00</p6><p7>0.00</p7><p8>0.00</p8><p9>0.00</p9><p10>0.00</p10><p11>0.00</p11><p12>0.00</p12><p13>0.00</p13><p14>0.00</p14><p29>0.00</p29></B>)"
		        uR"(<C><p1>7500.00</p1><p4>0.00</p4></C>)"
		        uR"(<D><p1>0.00</p1><p2>0.00</p2><p3>0.00</p3><p4>0.00</p4></D>)"
		        uR"(</III>)"
		        uR"(<IV><p1>%1</p1></IV>)"
		        uR"(</ZUSRCA>)"
		        uR"(<ZUSRCA id_dokumentu="4">)"
		        uR"(<I><p1><p1>01</p1><p2>2020-05</p2></p1></I>)"
		        uR"(<II><p1>7680002466</p1><p3>26211012346</p3><p4>1</p4><p5>ABC523456</p5><p6>JAN NOWAK</p6><p7>NOWAK</p7><p8>JAN</p8><p9>2026-01-10</p9></II>)"
		        uR"(<III id_bloku="1">)"
		        uR"(<A><p1>IKSIŃSKA</p1><p2>ANTONIA</p2><p3>P</p3><p4>00000000000</p4></A>)"
		        uR"(<B><p1><p1>0110</p1><p2>1</p2><p3>1</p3></p1><p3><p1>1</p1><p2>1</p2></p3><p4>0.00</p4><p5>0.00</p5><p6>0.00</p6><p7>0.00</p7><p8>0.00</p8><p9>0.00</p9><p10>0.00</p10><p11>0.00</p11><p12>0.00</p12><p13>0.00</p13><p14>0.00</p14><p29>0.00</p29></B>)"
		        uR"(<C><p1>0.00</p1><p4>0.00</p4></C>)"
		        uR"(<D><p1>0.00</p1><p2>0.00</p2><p3>0.00</p3><p4>0.00</p4></D>)"
		        uR"(</III>)"
		        uR"(<IV><p1>%1</p1></IV>)"
		        uR"(</ZUSRCA>)"
		        uR"(<ZUSDRA id_dokumentu="5">)"
		        uR"(<I><p1>6</p1><p2><p1>01</p1><p2>2020-05</p2></p2></I>)"
		        uR"(<II><p1>7680002466</p1><p3>26211012346</p3><p4>1</p4><p5>ABC523456</p5><p6>JAN NOWAK</p6><p7>NOWAK</p7><p8>JAN</p8><p9>2026-01-10</p9></II>)"
		        uR"(<III><p1>4</p1><p3>0.00</p3></III>)"
		        uR"(<IV><p1>0.00</p1><p2>0.00</p2><p3>0.00</p3><p4>0.00</p4><p5>0.00</p5><p6>0.00</p6><p7>0.00</p7><p8>0.00</p8><p9>0.00</p9><p10>0.00</p10><p11>0.00</p11><p12>0.00</p12><p13>0.00</p13><p14>0.00</p14><p15>0.00</p15><p16>0.00</p16><p17>0.00</p17><p18>0.00</p18><p19>0.00</p19><p20>0.00</p20><p21>0.00</p21><p22>0.00</p22><p23>0.00</p23><p24>0.00</p24><p25>0.00</p25><p26>0.00</p26><p27>0.00</p27><p28>0.00</p28><p29>0.00</p29><p30>0.00</p30><p31>0.00</p31><p32>0.00</p32><p33>0.00</p33><p34>0.00</p34><p35>0.00</p35><p36>0.00</p36><p37>0.00</p37></IV>)"
		        uR"(<V><p1>0.00</p1><p2>0.00</p2><p3>0.00</p3><p4>0.00</p4><p5>0.00</p5></V>)"
		        uR"(<VI><p2>0.00</p2><p5>0.00</p5><p6>0.00</p6><p7>0.00</p7></VI>)"
		        uR"(<VII><p1>0.00</p1><p2>0.00</p2><p3>0.00</p3></VII>)"
		        uR"(<VIII><p3>0.00</p3></VIII>)"
		        uR"(<IX><p1>0.00</p1><p2>0.00</p2></IX>)"
		        uR"(<XIII><p1>%1</p1></XIII>)"
		        uR"(</ZUSDRA>)"
		        uR"(</KEDU>)"}
		        .arg(QString::fromLatin1(today_s));
		QCOMPARE_EQ(qFileContents("here.xml"), text);
	}

	{
		static constexpr auto xml_args =
		    std::array{"--config"sv, "./.quick_dra.yaml"sv, "-m"sv, "0"sv, "--today"sv, "2020-05-01"sv};
		std::vector<char*> args{};
		args.reserve(xml_args.size() + 1);
		std::transform(xml_args.begin(), xml_args.end(), std::back_inserter(args),
		               [](auto const& sv) { return const_cast<char*>(sv.data()); });
		args.push_back(nullptr);
		builtin::xml::handle("qdra xml"sv, {static_cast<int>(xml_args.size()), args.data()}, ""sv);
		auto text = qFileContents("quick-dra_202005-01.xml");
		text.replace("2020-05-01", today_s.c_str());
		QCOMPARE_EQ(qFileContents("here.xml"), text);
	}

	{
		page.reportIdAccepted(2, QDate{2024y / April / 1}, true);
		QSettings settings{"config.ini", QSettings::IniFormat};
		settings.beginGroup("Settings");
		QCOMPARE_EQ(settings.value("ReportIndex"), QVariant{2u});
		QCOMPARE_EQ(settings.value("YearMonth"), QString{"2024/4"});
	}

	QVERIFY_SURVIVES_RELOAD();
}
