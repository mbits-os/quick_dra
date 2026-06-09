// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QMainWindow>

#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QTest>
#include <app/gui/CurrentColor.hpp>
#include <app/utils/utils.hpp>
#include <array>
#include <format>
#include <quick_dra/base/str.hpp>
#include "GuiTest.hpp"

using namespace quick_dra::gui;
using namespace quick_dra;

static constexpr auto iconCreators = std::array{
    gui::arrowRightSVGIcon, gui::arrowLeftSVGIcon, gui::ellipsisSVGIcon, gui::checkSVGIcon,
    gui::nullSVGIcon,       gui::resetSVGIcon,     gui::warningSVGIcon,
};

class CurrentColorWidget : public QWidget {
public:
	CurrentColorWidget() {
		auto it = icons.begin();
		for (auto const& create : iconCreators) {
			*it++ = create();
		}
		int width = 0;
		int height = 2 * margin;

		for (auto const& icon : icons) {
			auto const size = icon.actualSize({16, 16});
			width += margin + size.width();
			auto const h = 2 * margin + size.height();
			height = std::max(h, height);
		}
		width += margin;
		resize(std::max(width, 2 * margin), height);
	}

	void paintEvent(QPaintEvent*) override {
		QPainter painter{this};
		auto const h = height();
		auto x = margin;
		for (auto const& icon : icons) {
			auto const size = icon.actualSize({16, 16});
			auto const y = (h - size.height()) / 2;

			icon.paint(&painter, QRect{x, y, size.width(), size.height()}, Qt::AlignCenter,
			           isEnabled() ? QIcon::Active : QIcon::Disabled);

			x += margin + size.width();
		}
	}

	template <size_t Index>
	QIcon copy() const {
		if constexpr (Index <= iconCreators.size()) {
			return icons[Index];
		}
	}

private:
	static constexpr auto margin = 6;
	std::array<QIcon, iconCreators.size()> icons{};
};

// #include "CurrentColor.test.moc"

struct testcase {
	QColor window;
	QColor windowText;

	void setPalette(auto* target) const {
		auto const lightness = (window.toHsl().lightnessF() + windowText.toHsl().lightnessF() * 2) / 3;
		auto const disabled = QColor::fromRgbF(lightness, lightness, lightness, windowText.alphaF()).toRgb();

		auto palette = target->palette();
		palette.setColor(QPalette::Normal, QPalette::Window, window);
		palette.setColor(QPalette::Normal, QPalette::WindowText, windowText);
		palette.setColor(QPalette::Disabled, QPalette::Window, window);
		palette.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
		target->setPalette(palette);
	}
};

static testcase const tests[] = {
    {.window = Qt::lightGray, .windowText = Qt::black},
    {.window = Qt::black, .windowText = Qt::white},
    {.window = QColor{40, 0, 0}, .windowText = QColor{255, 200, 0, 192}},
};

QImage generateDifferenceMap(QImage const& baseline, QImage const& grabbed) {
	// Ensure both images are a standard format, like 32-bit ARGB
	QImage expected = baseline.convertToFormat(QImage::Format_ARGB32);
	QImage actual = grabbed.convertToFormat(QImage::Format_ARGB32);

	auto const error = qRgba(255, 0, 0, 255);

	QImage diffImage{std::max(expected.width(), actual.width()), std::max(expected.height(), actual.height()),
	                 QImage::Format_ARGB32};
	diffImage.fill(Qt::transparent);
	if (expected.width() != actual.width()) {
		auto const from = std::min(expected.width(), actual.width());
		auto const to = std::max(expected.width(), actual.width());
		QPainter{&diffImage}.fillRect(QRect{from, 0, to - from, diffImage.height()}, error);
	}
	if (expected.height() != actual.height()) {
		auto const from = std::min(expected.height(), actual.height());
		auto const to = std::max(expected.height(), actual.height());
		QPainter{&diffImage}.fillRect(QRect{0, from, diffImage.width(), to - from}, error);
	}

	int width = std::min(expected.width(), actual.width());
	int height = std::min(expected.height(), actual.height());

	for (int y = 0; y < height; ++y) {
		const QRgb* line1 = reinterpret_cast<const QRgb*>(expected.constScanLine(y));
		const QRgb* line2 = reinterpret_cast<const QRgb*>(actual.constScanLine(y));
		QRgb* diffLine = reinterpret_cast<QRgb*>(diffImage.scanLine(y));

		for (int x = 0; x < width; ++x) {
			if (line1[x] != line2[x]) {
				// Highlight differences in red
				diffLine[x] = qRgba(255, 0, 0, 255);
			} else {
				// Dim the matching pixels
				QRgb original = line1[x];
				int gray = qGray(original);
				diffLine[x] = qRgba(gray, gray, gray, 255);
			}
		}
	}
	return diffImage;
}

void GuiTest::CurrentColor_changePaletteAtRuntime() {
	CurrentColorWidget widget{};

	auto const size = widget.size() * 2;

	QImage testedImage{size.width() * 2, size.height() * static_cast<int>(std::size(tests)), QImage::Format_RGB32};

	{
		QPainter image{&testedImage};

		int y = 0;

		for (auto const& test : tests) {
			test.setPalette(qApp);
			test.setPalette(&widget);
			for (auto const enabled : {true, false}) {
				widget.setEnabled(enabled);
				auto x = enabled ? 0 : size.width();
				image.drawImage(QRect{x, y, size.width(), size.height()}, widget.grab().toImage());
			}
			y += size.height();
		}
	}

	auto const images_dir = std::filesystem::path{as_u8v(__FILE__)}.parent_path() / "images";
	std::filesystem::create_directories(images_dir);

	auto const image_path = images_dir / "CurrentColor.png";
	auto const path = QString::fromUtf8(as_sv(image_path.u8string()));
	if (!std::filesystem::exists(image_path)) {
		qWarning() << "Rebuilding images/CurrentColor.png";
		testedImage.save(path);
	}

	QImage stencil{};
	stencil.load(path);
	try {
		QTest::ThrowOnFailEnabler failGuard{};
		QCOMPARE_EQ(testedImage, stencil);
	} catch (...) {
		auto const error_image_path = images_dir / "CurrentColor.error.png";
		auto const error_path = QString::fromUtf8(as_sv(error_image_path.u8string()));
		generateDifferenceMap(stencil, testedImage).save(error_path);
		throw;
	}
}
