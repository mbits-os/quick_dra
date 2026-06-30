// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QPainter>
#include <QTest>
#include <app/controls/WordWrap.hpp>
#include <array>
#include <filesystem>
#include <print>
#include <quick_dra/base/str.hpp>
#include <source_location>
#include <string_view>
#include "ControlsTest.hpp"
#include "stamper.hpp"

using namespace quick_dra::gui;
using namespace quick_dra;
using namespace std::literals;

struct FormattedLabel {
	MarkdownLabel label{};
	static constexpr auto margin = qreal{5};

	void setWidth(int width) { label.setWidth(width); }

	RenderCompare render(std::string_view filename, qreal dpScale = 4) {
		auto size = label.calcSize().grownBy({margin, margin, margin, margin});
		label.moveTo({margin, margin});
		Painter renderer{size.toSize(), filename, dpScale};
		renderer.paint(label);
		return RenderCompare{std::move(renderer)};
	}

private:
	class Painter : public RenderCompare {
	public:
		Painter(QSize const& size, std::string_view filename, qreal dpScale)
		    : RenderCompare{{.path = filename, .size = size, .dpScale = dpScale, .background = Qt::white}} {}

		QPainter get() { return painter(); }

		void paint(MarkdownLabel& label) {
			auto image = painter();
			image.fillRect({QPointF{margin, margin}, label.calcSize()}, QColor{0, 0, 0, 16});
			label.paint(&image);
		}
	};
};

inline QString operator""_s(char16_t const* text, size_t count) {
	return QString::fromUtf16(text, static_cast<qsizetype>(count));
}
constexpr inline QStringView operator""_sv(char16_t const* text, size_t count) {
	return {text, static_cast<qsizetype>(count)};
}

using test_pair = std::pair<std::u16string_view, std::string_view>;

static constexpr auto lorem_ipsum =
    u"Just some *line.* **Lorem ipsum dolor sit amet,** consectetur adipiscing elit, sed do eiusmod tempor incididunt "
    u"ut ***labore*** et dolore magna aliqua. Ut enim ad **mi***nim* veniam, quis nostrud exercitation ullamco laboris "
    u"nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum "
    u"dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia "
    u"deserunt mollit anim id est laborum."_sv;

FormattedLabel sampleText() {
	FormattedLabel sample{};
	sample.label.setFont(qApp->font());
	sample.label.setText(QString{lorem_ipsum});

	return sample;
}

void ControlsTest::SplitSpaces_data() {
	QTest::addColumn<QString>("text");
	QTest::addColumn<QStringList>("expected");

	QTest::newRow("empty") << u""_s << QStringList{};
	QTest::newRow("single word") << u"singleword"_s << QStringList{u"singleword"_s};
	QTest::newRow("two words") << u"two words"_s << QStringList{u"two"_s, u"words"_s};

	static constexpr auto spaces = std::array{
	    test_pair{u"\u0009"sv, "character tabulation"sv},
	    test_pair{u"\u000A"sv, "line feed"sv},
	    test_pair{u"\u000B"sv, "line tabulation"sv},
	    test_pair{u"\u000C"sv, "form feed"sv},
	    test_pair{u"\u000D"sv, "carriage return"sv},
	    test_pair{u"\u0020"sv, "space"sv},
	    test_pair{u"\u0085"sv, "next line"sv},
	    test_pair{u"\u1680"sv, "ogham space mark"sv},
	    test_pair{u"\u2000"sv, "en quad"sv},
	    test_pair{u"\u2001"sv, "em quad"sv},
	    test_pair{u"\u2002"sv, "en space"sv},
	    test_pair{u"\u2003"sv, "em space"sv},
	    test_pair{u"\u2004"sv, "three-per-em space"sv},
	    test_pair{u"\u2005"sv, "four-per-em space"sv},
	    test_pair{u"\u2006"sv, "six-per-em space"sv},
	    test_pair{u"\u2007"sv, "figure space"sv},
	    test_pair{u"\u2008"sv, "punctuation space"sv},
	    test_pair{u"\u2009"sv, "thin space"sv},
	    test_pair{u"\u200A"sv, "hair space"sv},
	    test_pair{u"\u2028"sv, "line separator"sv},
	    test_pair{u"\u2029"sv, "paragraph separator"sv},
	    test_pair{u"\u205F"sv, "medium mathematical space"sv},
	    test_pair{u"\u3000"sv, "ideographic space"sv},
	};

	for (auto const& [space, name] : spaces) {
		QTest::newRow(name.data()) << QString{space} << QStringList{};
	}

	static constexpr auto non_spaces = std::array{
	    test_pair{u"\u00A0"sv, "no-break space"sv},
	    test_pair{u"\u202F"sv, "narrow no-break space"sv},
	    test_pair{u"\u180E"sv, "mongolian vowel separator"sv},
	    test_pair{u"\u200B"sv, "zero width space"sv},
	    test_pair{u"\u200C"sv, "zero width non-joiner"sv},
	    test_pair{u"\u200D"sv, "zero width joiner"sv},
	    test_pair{u"\u2060"sv, "word joiner"sv},
	    test_pair{u"\uFEFF"sv, "zero width non-breaking space"sv},
	};

	for (auto const& [space, name] : non_spaces) {
		QTest::newRow(name.data()) << QString{space} << QStringList{QString{space}};
	}

	QTest::newRow("some variation") << u" \u2008label:\t\"value\",   \rlabel:\u2003\u20031\u00A0345\uFEFFzł\u200A \n"_s
	                                << QStringList{u"label:"_s, u"\"value\","_s, u"label:"_s, u"1\u00A0345\uFEFFzł"_s};
}

void ControlsTest::SplitSpaces() {
	QFETCH(QString, text);
	QFETCH(QStringList, expected);

	auto const spans = splitForWordWrap(text);

	QStringList actual{};
	actual.reserve(spans.size());
	std::transform(spans.begin(), spans.end(), std::back_inserter(actual),
	               [&text](StringSpan const& span) { return span.sliceInto(text).toString(); });

	QCOMPARE_EQ(actual, expected);
}

static consteval StringSpan sspan(qsizetype offset, qsizetype length) { return {.offset = offset, .length = length}; }

void ControlsTest::InlineMarkdown_data() {
	QTest::addColumn<QString>("text");
	QTest::addColumn<QString>("expected");
	QTest::addColumn<QList<StringSpan>>("bold");
	QTest::addColumn<QList<StringSpan>>("italic");

	auto const empty = QList<StringSpan>{};

	QTest::newRow("empty") << u""_s << u""_s << empty << empty;
	QTest::newRow("simple bold") << u"before **dur__ing__ after** __disjointed__"_s
	                             << u"before during after disjointed"_s << QList{sspan(7, 12), sspan(20, 10)} << empty;
	QTest::newRow("simple bold reversed")
	    << u"before __dur**ing__ after** __disjointed__"_s << u"before during after disjointed"_s
	    << QList{sspan(7, 12), sspan(20, 10)} << empty;
	QTest::newRow("unfinished") << u"before **dur__ing__** after __unfinished"_s
	                            << u"before during after __unfinished"_s << QList{sspan(7, 6)} << empty;
	QTest::newRow("balding italian") << u"before **dur*ing*** after *unfinished"_s
	                                 << u"before during after *unfinished"_s << QList{sspan(7, 5)}
	                                 << QList{sspan(10, 3)};
	QTest::newRow("triple into italic+bold")
	    << u"***both* only bold**"_s << u"both only bold"_s << QList{sspan(0, 14)} << QList{sspan(0, 4)};
	QTest::newRow("restarted bold") << u"____bold__"_s << u"__bold"_s << QList{sspan(2, 4)} << empty;
	QTest::newRow("restarted italic") << u"**italic*"_s << u"*italic"_s << empty << QList{sspan(1, 6)};
	QTest::newRow("**mi***nim*") << u"**mi***nim*"_s << u"minim"_s << QList{sspan(0, 2)} << QList{sspan(2, 3)};
	QTest::newRow("***labore***") << u"***labore***"_s << u"labore"_s << QList{sspan(0, 6)} << QList{sspan(0, 6)};
}

void ControlsTest::InlineMarkdown() {
	QFETCH(QString, text);
	QFETCH(QString, expected);
	QFETCH(QList<StringSpan>, bold);
	QFETCH(QList<StringSpan>, italic);

	auto formatted = markdownFormat(text);

	try {
		QTest::ThrowOnFailEnabler guard{};
		QCOMPARE_EQ(formatted.text, expected);
		QCOMPARE_EQ(formatted.bold, bold);
		QCOMPARE_EQ(formatted.italic, italic);
	} catch (...) {
#define PRINT(STYLE)                                                   \
	do {                                                               \
		if (formatted.STYLE.empty()) {                                 \
			std::print(" << empty");                                   \
		} else {                                                       \
			std::print(" << QList{{");                                 \
			auto first = true;                                         \
			for (auto const& span : formatted.STYLE) {                 \
				if (first)                                             \
					first = false;                                     \
				else                                                   \
					std::print(", ");                                  \
				std::print("sspan({}, {})", span.offset, span.length); \
			}                                                          \
			std::print("}}");                                          \
		}                                                              \
	} while (false)
		std::print("[{}] u\"{}\"_s", QTest::currentDataTag(), formatted.text.toStdString());
		PRINT(bold);
		PRINT(italic);
		std::print("\n");
		throw;
	}
}

void ControlsTest::MeasureText_data() {
	QTest::addColumn<int>("width");
	QTest::addColumn<QSizeF>("expected");
	QTest::newRow("max px") << std::numeric_limits<int>::max() << QSizeF{2807, 14};
#define NEW_ROW(width, size_w, size_h)   /**/ \
	QTest::newRow(#width " px") << width /**/ \
	                            << QSizeF{size_w, size_h}
	NEW_ROW(1000, 998, 42);
	NEW_ROW(250, 250, 182);
	NEW_ROW(100, 100, 448);
#undef NEW_ROW
}

void ControlsTest::MeasureText() {
	auto sample = sampleText();

	QFETCH(int, width);
	QFETCH(QSizeF, expected);

	sample.setWidth(width);

	try {
		QTest::ThrowOnFailEnabler failGuard{};
		QCOMPARE_EQ(sample.label.calcSize(), expected);
	} catch (...) {
		sample.render(std::format("images/MeasureText-{}.png", QTest::currentDataTag())).storeTested();
		throw;
	}
}

void ControlsTest::PaintMarkdown() {
	auto sample = sampleText();

	sample.setWidth(250);
	{
		auto canvas = sample.render("images/PaintMarkdown_" OS_NAME ".png"sv);
		auto stencil = canvas.loadStencil();

		COMPARE_IMAGES(stencil, canvas);
	}

	sample.label.setIsHeading(true);
	{
		auto canvas = sample.render("images/PaintMarkdown-heading_" OS_NAME ".png"sv);
		auto stencil = canvas.loadStencil();

		COMPARE_IMAGES(stencil, canvas);
	}
}

void ControlsTest::FindSmallestRect() {
	auto sample = sampleText();
	auto const size = sample.label.calcNarrowest();

	try {
		QTest::ThrowOnFailEnabler failGuard{};
		QCOMPARE_EQ(size.width(), 83);
		QCOMPARE_EQ(size.height(), 1008);
	} catch (...) {
		sample.render(std::format("images/FindSmallestRect.png", QTest::currentDataTag())).storeTested();
		throw;
	}
}