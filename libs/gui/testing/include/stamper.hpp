// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QColor>
#include <QImage>
#include <QMargins>
#include <QString>
#include <QWidget>
#include <filesystem>
#include <source_location>

class Stamper {
public:
	struct Options {
		std::string_view path{};
		int columns{1};
		int rows{1};
		QMargins margins{};
		int spacing{0};
		QImage::Format format{QImage::Format_RGB32};
		QColor background{format == QImage::Format_ARGB32 ? Qt::transparent : Qt::black};
	};

	struct Position {
		int row{};
		int column{};
		bool operator==(Position const&) const noexcept = default;
	};

	struct Iterator {
		int columns{};
		Position curr{};
		bool operator==(Iterator const&) const noexcept = default;
		Position const& operator*() const noexcept { return curr; }
		Iterator& operator++() noexcept {
			curr.column = (curr.column + 1) % columns;
			if (!curr.column) ++curr.row;
			return *this;
		}
	};

	Stamper(QWidget* target, Options const& options, std::source_location const& = std::source_location::current());

	QString const& imageTitle() const noexcept { return imageTitle_; }
	Iterator begin() const noexcept { return {.columns = columns_}; }
	Iterator end() const noexcept { return {.columns = columns_, .curr = {.row = rows_, .column = 0}}; }

	void grab(int row, int column);
	void grab(Position const& pos) { grab(pos.row, pos.column); }

	QImage loadStencil(bool always = false);
	QImage const& testedImage() const noexcept { return tested_; }

	void saveDiffMap(QImage const& stencil);

private:
	QWidget* target_;
	std::filesystem::path imagePath_;
	QString imageTitle_;
	int columns_;
	int rows_;
	QMargins margins_;
	int spacing_;
	QImage tested_;
	QSize widgetSize_{target_->size() * 2};
};

#define COMPARE_IMAGES(STENCIL, STAMPER)             \
	try {                                            \
		QTest::ThrowOnFailEnabler failGuard{};       \
		QCOMPARE_EQ(STAMPER.testedImage(), STENCIL); \
	} catch (...) {                                  \
		STAMPER.saveDiffMap(STENCIL);                \
		throw;                                       \
	}

inline auto safe_size_t(int pos) { return static_cast<size_t>(pos < 0 ? 0 : pos); }
