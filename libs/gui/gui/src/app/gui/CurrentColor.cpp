// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QApplication>
#include <QColor>
#include <QFile>
#include <QIconEngine>
#include <QPainter>
#include <QPalette>
#include <QSvgRenderer>
#include <app/gui/CurrentColor.hpp>
#include <format>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string_view>
#include <utility>

using namespace std::literals;

namespace quick_dra::gui {
	struct SimplePalette {
		QColor normal{Qt::transparent};
		QColor disabled{Qt::transparent};
		bool operator==(SimplePalette const&) const noexcept = default;

		static SimplePalette fromApp() {
			auto const palette = qApp->palette();
			return {
			    .normal = palette.color(QPalette::Normal, QPalette::WindowText),
			    .disabled = palette.color(QPalette::Disabled, QPalette::WindowText),
			};
		}
	};

	class SVGIconEngine : public QIconEngine {
	public:
		struct SharedState {
			SharedState(QString const& templatePath) : svgTemplate{loadTemplateText(templatePath)} {
				updateIconsLocked();
			}

			QSize defaultSize() const noexcept { return normalRenderer.defaultSize(); }
			void paint(QPainter* painter, QRect const& rect, QIcon::Mode mode) {
				updateIconsUnlocked();

				std::shared_lock readLock{accessMutex};
				if (mode == QIcon::Disabled) {
					disabledRenderer.render(painter, rect);
				} else {
					normalRenderer.render(painter, rect);
				}
			}

		private:
			static QByteArray loadTemplateText(QString const& path) {
				auto file = QFile{path};
				if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
					return {};
				}

				return file.readAll();
			}

			QByteArray replaceCurrentColor(QColor color) {
				auto const buffer = color.name(QColor::HexRgb).toUtf8();
				auto const colorName = std::format("\"{}\"", std::string_view{buffer});
				auto const alpha = color.alphaF();
				auto bytes = svgTemplate;
				if (alpha < 1) {
					bytes.replace(" viewBox=\"", std::format(" opacity=\"{}\" viewBox=\"", alpha));
				}

				bytes.replace("\"currentColor\""sv, colorName);
				return bytes;
			}

			void updateIconsUnlocked() {
				if (SimplePalette::fromApp() == currentColor) return;

				std::unique_lock modifyLock{accessMutex};
				auto const appColor = SimplePalette::fromApp();
				if (appColor == currentColor) return;
				currentColor = appColor;
				updateIconsLocked();
			}

			void updateIconsLocked() {
				normalRenderer.load(replaceCurrentColor(currentColor.normal));
				disabledRenderer.load(replaceCurrentColor(currentColor.disabled));
				size = normalRenderer.defaultSize();
			}

			std::shared_mutex accessMutex{};
			QByteArray svgTemplate;
			SimplePalette currentColor{SimplePalette::fromApp()};
			QSvgRenderer normalRenderer{};
			QSvgRenderer disabledRenderer{};
			QSize size{};
		};

		// explicit SVGIconEngine(QString const& templatePath) : state_{std::make_shared<SharedState>(templatePath)} {}
		explicit SVGIconEngine(std::shared_ptr<SharedState> copy) : state_{std::move(copy)} {}

		QSize actualSize(const QSize&, QIcon::Mode, QIcon::State) override { return state_->defaultSize(); }
		QIconEngine* clone() const override { return new SVGIconEngine{state_}; }
		QPixmap pixmap(QSize const& size, QIcon::Mode mode, QIcon::State state) override {
			QImage img(size, QImage::Format_ARGB32);
			img.fill(qRgba(0, 0, 0, 0));
			QPixmap pix = QPixmap::fromImage(img, Qt::NoFormatConversion);
			this->paint(&pix, {{}, size}, mode, state);
			return pix;
		}
		void paint(QPixmap* pix, QRect const& rect, QIcon::Mode mode, QIcon::State state) {
			QPainter painter{pix};
			paint(&painter, rect, mode, state);
		}
		void paint(QPainter* painter, QRect const& rect, QIcon::Mode mode, QIcon::State) override {
			state_->paint(painter, rect, mode);
		}

	private:
		std::shared_ptr<SharedState> state_{};
	};

	class IconCache {
	public:
		static IconCache& instance() noexcept {
			static IconCache cache{};
			return cache;
		}

		QIcon currentColorIcon(QString const& path) { return QIcon{new SVGIconEngine{findIcon(path)}}; }

	private:
		std::shared_ptr<SVGIconEngine::SharedState> findIcon(QString const& path) {
			{
				std::shared_lock tree_reading{insertMutex};
				auto it = loadedIcons.find(path);
				if (it != loadedIcons.end()) return it->second;
			}

			std::unique_lock tree_modifying{insertMutex};
			auto it = loadedIcons.lower_bound(path);
			if (it != loadedIcons.end() && it->first == path) {
				// someone already did it
				return it->second;
			}

			auto state = std::make_shared<SVGIconEngine::SharedState>(path);
			it = loadedIcons.insert(it, {path, std::move(state)});
			return it->second;
		}

		std::shared_mutex insertMutex{};
		std::map<QString, std::shared_ptr<SVGIconEngine::SharedState>> loadedIcons{};
	};

	QIcon arrowRightSVGIcon() { return IconCache::instance().currentColorIcon(":/icons/arrow_right.svg"); }
	QIcon arrowLeftSVGIcon() { return IconCache::instance().currentColorIcon(":/icons/arrow_left.svg"); }
	QIcon ellipsisSVGIcon() { return IconCache::instance().currentColorIcon(":/icons/ellipsis.svg"); }
	QIcon checkSVGIcon() { return IconCache::instance().currentColorIcon(":/icons/check.svg"); }
	QIcon nullSVGIcon() { return IconCache::instance().currentColorIcon(":/icons/null.svg"); }
};  // namespace quick_dra::gui
