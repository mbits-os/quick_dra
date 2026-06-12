// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "stamper.hpp"
#include <QPainter>
#include <quick_dra/base/str.hpp>

using namespace std::literals;

namespace {
	std::filesystem::path buildTarget(std::string_view path, std::source_location const& here) {
		return weakly_canonical(std::filesystem::path{here.file_name()}.parent_path() / path);
	}

	auto clip(auto v) { return v < 0 ? 0 : v > 1 ? 1 : v; }

	double distanceF(QRgb rgb1, QRgb rgb2) {
		auto const deltaRed = qRed(rgb1) - qRed(rgb2);
		auto const deltaGreen = qGreen(rgb1) - qGreen(rgb2);
		auto const deltaBlue = qBlue(rgb1) - qBlue(rgb2);
		auto const distSquared = deltaRed * deltaRed + deltaGreen * deltaGreen + deltaBlue * deltaBlue;
		auto const dist = std::sqrt(distSquared);
		static constexpr auto maxDist = 441.67295593;  // std::sqrt(3 * 255 * 255);
		return clip(dist / maxDist);
	}

	int scale(double pos, int from, int to) { return static_cast<int>(static_cast<double>(from) + pos * (to - from)); }

	QRgb distance(double dist) { return qRgba(255 * 3 / 5, scale(dist, 192, 0) * 3 / 5, 0, 255); }
	QRgb distance(QRgb rgb1, QRgb rgb2) { return distance(distanceF(rgb1, rgb2)); }

	QImage generateDifferenceMap(QImage const& baseline, QImage const& grabbed) {
		// Ensure both images are a standard format, like 32-bit ARGB
		QImage expected = baseline.convertToFormat(QImage::Format_ARGB32);
		QImage actual = grabbed.convertToFormat(QImage::Format_ARGB32);

		auto const error = qRgba(255, 0, 0, 255);

		QImage diffImage{std::max(expected.width(), actual.width()), std::max(expected.height(), actual.height()) + 2,
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
					// Highlight differences in orange-to-red
					diffLine[x] = distance(line1[x], line2[x]);
				} else {
					// Dim the matching pixels
					auto const original = line1[x];
					auto const gray = qGray(original);
					diffLine[x] = qRgba(0, gray * 3 / 5, 0, qAlpha(original));
				}
			}
		}

		QRgb* diffLine = reinterpret_cast<QRgb*>(diffImage.scanLine(std::max(expected.height(), actual.height()) + 1));
		int points = std::max(expected.width(), actual.width());
		for (int x = 0; x < points; ++x) {
			diffLine[x] = distance(static_cast<double>(x) / points);
		}
		return diffImage;
	}
}  // namespace

Stamper::Stamper(QWidget* target, Options const& options, std::source_location const& here)
    : target_{target}
    , imagePath_{buildTarget(options.path, here)}
    , imageTitle_{QString::fromUtf8(options.path)}
    , columns_{options.columns}
    , rows_{options.rows}
    , margins_{options.margins}
    , spacing_{options.spacing} {
	auto const width = (margins_.left() + margins_.right()) + (widgetSize_.width() + spacing_) * columns_ - spacing_;
	auto const height = (margins_.top() + margins_.bottom()) + (widgetSize_.height() + spacing_) * rows_ - spacing_;
	tested_ = QImage{width, height, options.format};
	tested_.fill(options.background);
}

void Stamper::grab(int row, int column) {
	if (row >= rows_ || column >= columns_) {
		return;
	}

	auto const x = margins_.left() + (widgetSize_.width() + spacing_) * column;
	auto const y = margins_.top() + (widgetSize_.height() + spacing_) * row;

	QPainter image{&tested_};
	image.drawImage(QRect{x, y, widgetSize_.width(), widgetSize_.height()}, target_->grab().toImage());
}

struct noquote_t {
	friend QDebug operator<<(QDebug out, noquote_t) {
		out.noquote();
		return out;
	}
};

static constexpr noquote_t noquote{};

QImage Stamper::loadStencil(bool always) {
	std::filesystem::create_directories(imagePath_.parent_path());
	auto const path = QString::fromUtf8(quick_dra::as_sv(imagePath_.u8string()));
	if (always || !std::filesystem::exists(imagePath_)) {
		qWarning() << noquote << "Rebuilding" << imageTitle_;
		tested_.save(path);
	}

	QImage stencil{};
	stencil.load(path);
	return stencil;
}

void Stamper::saveDiffMap(QImage const& stencil) {
	auto const filename =
	    imagePath_.stem().generic_u8string() + u8".error"s + imagePath_.extension().generic_u8string();
	auto const error_path = imagePath_.parent_path() / filename;
	auto const path = QString::fromUtf8(quick_dra::as_sv(error_path.u8string()));
	generateDifferenceMap(stencil, tested_).save(path);
}
