// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QToolBar>
#include <algorithm>
#include <app/gui/CurrentColor.hpp>
#include <app/gui/PageHeader.hpp>
#include <app/utils/LaidOut.hpp>
#include <app/utils/utils.hpp>
#include <array>
#include <memory>
#include <string_view>

using namespace std::literals;

namespace quick_dra::gui {
	namespace {
		QAction* createAction(QObject* parent, std::string_view name, std::string_view toolTip, QIcon const& icon) {
			auto result = std::make_unique<QAction>(parent);
			result->setObjectName(QString::fromUtf8(name));
			result->setIcon(icon);
			result->setToolTip(QString::fromUtf8(toolTip));
			result->setMenuRole(QAction::MenuRole::NoRole);
			return result.release();
		}
		void setupAction(QToolBar* bar, QAction* action) {
			auto widget = bar->widgetForAction(action);
			restrictToolButton(widget, 36);
		}
	}  // namespace

	void HeaderToolbar::resizeEvent(QResizeEvent* event) {
		QToolBar::resizeEvent(event);

		auto const current = width();
		if (current == width_) {
			return;
		}

		width_ = current;
		widthChanged(width_);
	}

	QSize HeaderSpacer::sizeHint() const { return {std::max(hintWidth_, 1), 1}; }

	QSize HeaderSpacer::minimumSizeHint() const { return HeaderSpacer::sizeHint(); }

	void HeaderSpacer::setHintWidth(int value) {
		hintWidth_ = value;
		updateGeometry();
	}

	QSize HeaderShadow::sizeHint() const { return {1, std::max(shadowHeight_, 1)}; }

	QSize HeaderShadow::minimumSizeHint() const { return HeaderShadow::sizeHint(); }

	static constexpr auto gauss = std::array{
	    1.0f,         0.945959469f, 0.800737403f, 0.60653066f,  0.411112291f, 0.249352209f,
	    0.135335283f, 0.065728529f, 0.028565501f, 0.011108997f, 0.00386592f,
	};

	float gauss_scaling(float length, float pos) {
		auto const size = static_cast<float>(gauss.size());
		auto const scaled_pos = pos * size / length;
		auto const index_low = static_cast<size_t>(scaled_pos);
		auto const index_high = index_low + 1;

		if (index_low <= 0) return gauss.front();
		if (index_high >= gauss.size()) return gauss.back();

		auto const diff = gauss[index_high] - gauss[index_low];
		return gauss[index_low] + diff * (scaled_pos - static_cast<float>(index_low));
	}

	float gauss_scaling(int length, int pos) {
		return gauss_scaling(static_cast<float>(length), static_cast<float>(pos));
	}

	QColor getOsAccent(QPalette const& palette) { return palette.color(QPalette::Highlight); }
	QColor addAlpha(QColor color, int alpha) {
		color.setAlpha(alpha);
		return color;
	}

	void HeaderShadow::paintEvent(QPaintEvent*) {
		QPainter p{this};
		auto const w = static_cast<float>(width());
		auto pos = 0.5f;

		p.setPen(addAlpha(palette().color(QPalette::Midlight), 128));
		p.drawLine(QPointF{0.0f, pos}, QPointF{w, pos});

		auto const lineCount = height() - 2;
		for (auto index = 0; index < lineCount; ++index) {
			auto const scale = gauss_scaling(lineCount, index) * shadowForce_;

			pos += 1;
			p.setPen(addAlpha(Qt::black, static_cast<int>(255 * scale + 0.5f)));
			p.drawLine(QPointF{0.0f, pos}, QPointF{w, pos});
		}
	}

	void HeaderShadow::setShadowHeight(int value) {
		shadowHeight_ = value;
		updateGeometry();
	}

	void HeaderShadow::setShadowForce(float value) {
		shadowForce_ = value;
		updateGeometry();
	}

	void HeaderShadow::targetMoved(QRect const& targetGeometry) {
		auto const pos = targetGeometry.bottomLeft();
		auto const size = QSize{targetGeometry.width(), shadowHeight_};
		setGeometry({pos, size});
	}

	PageHeader::PageHeader(QWidget* parent) : QWidget{parent} { setupUI(); }

	void PageHeader::leavePage() { navigatingBack(); }
	void PageHeader::acceptChanges() { changesAccepted(); }

	// PAGE STACK API
	void PageHeader::setFormDirty(bool value) {
		if (formDirty_ == value) {
			return;
		}
		formDirty_ = value;
		formDirtyChanged(formDirty_);
	}
	void PageHeader::setFormValid(bool value) {
		if (formValid_ == value) {
			return;
		}
		formValid_ = value;
		formValidChanged(formValid_);
	}
	void PageHeader::setTitle(QString const& title) { titleLabel_->setText(title); }
	void PageHeader::setTopMost(bool value) {
		if (topMost_ == value) {
			return;
		}
		topMost_ = value;
		showActionBack(!topMost_);
	}

	void PageHeader::resizeEvent(QResizeEvent* event) {
		QWidget::resizeEvent(event);
		moved(geometry());
	}

	void PageHeader::paintEvent(QPaintEvent*) {
		QPainter p{this};
		auto color = palette().color(QPalette::WindowText);
		color.setAlpha(color.alpha() * 10 / 255);
		if (!color.alpha()) {
			return;
		}

		auto const s = this->size();
		auto const width = static_cast<float>(s.width());
		auto pos = static_cast<float>(s.height()) - .5f;
		p.fillRect(QRectF{0.0f, 0.0f, width, pos + 1}, color);
	}

	void PageHeader::setupUI() {
		QHBoxLayout* horizontalLayout{};
		HeaderSpacer* spacer{};

		LaidOut{this}.createLayout(horizontalLayout, "horizontalLayout", this, [](QHBoxLayout& horizontalLayout) {
			horizontalLayout.setContentsMargins(0, 0, 0, 0);
			horizontalLayout.setSpacing(0);
		});
		auto root = LaidOut{this, horizontalLayout};

		root.createWidget(toolBar_, "toolBar", [self = this, horizontalLayout](QToolBar& toolBar) {
			toolBar.setIconSize({16, 16});
			toolBar.setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);

			auto actionBack = createAction(self, "actionBack"sv, "Cofnij"sv, arrowLeftSVGIcon());
			auto actionAccept = createAction(self, "actionAccept"sv, "Zastosuj"sv, checkSVGIcon());

			toolBar.addAction(actionBack);
			toolBar.addAction(actionAccept);

			setupAction(&toolBar, actionBack);
			setupAction(&toolBar, actionAccept);

			QObject::connect(actionBack, &QAction::triggered, self, &PageHeader::leavePage);
			QObject::connect(actionAccept, &QAction::triggered, self, &PageHeader::acceptChanges);
			QObject::connect(self, &PageHeader::showActionBack, actionBack, &QAction::setVisible);
			QObject::connect(self, &PageHeader::formDirtyChanged, actionAccept, &QAction::setVisible);
			QObject::connect(self, &PageHeader::formValidChanged, actionAccept, &QAction::setEnabled);
			actionAccept->setVisible(false);

			auto const m = horizontalLayout->contentsMargins();
			auto const size = toolBar.sizeHint().height() + m.top() + m.bottom();
			self->setMinimumSize(QSize(size, size));

			self->setSizePolicy(TakeWidth);
		});

		root.createWidget(titleLabel_, "titleLabel", [self = this](QLabel& titleLabel) {
			titleLabel.setSizePolicy(TakeAll / 1_XStretch);
			QFont font;
			font.setBold(true);
			font.setPointSizeF(font.pointSizeF() * 1.2);
			titleLabel.setFont(font);
			titleLabel.setAlignment(Qt::AlignmentFlag::AlignCenter);

#if 0
			auto pal = titleLabel.palette();
			auto shadow = pal.color(QPalette::WindowText);
			shadow.setAlpha(12);
			pal.setColor(QPalette::Window, shadow);
			titleLabel.setPalette(pal);
			titleLabel.setAutoFillBackground(true);
#endif
		});

		root.createWidget(spacer, "spacer", [self = this, toolBar = toolBar_](HeaderSpacer& spacer) {
			spacer.setSizePolicy(TakeHeight);
			QObject::connect(toolBar, &HeaderToolbar::widthChanged, &spacer, &HeaderSpacer::setHintWidth);
		});
	}
}  // namespace quick_dra::gui
