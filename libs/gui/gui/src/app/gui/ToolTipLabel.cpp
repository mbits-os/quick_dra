// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QApplication>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QStylePainter>
#include <QToolTip>
#include <app/gui/ToolTipLabel.hpp>
#include <utility>

namespace quick_dra::gui {
	ToolTipLabel::ToolTipLabel(const QString& value, const QPoint& pos, QWidget* parent)
	    : QLabel{parent, Qt::ToolTip | Qt::BypassGraphicsProxyWidget} {
		setFont(QFont{"QTipLabel"});
		setForegroundRole(QPalette::ToolTipText);
		setBackgroundRole(QPalette::ToolTipBase);
		setPalette(QToolTip::palette());
		ensurePolished();
		setMargin(1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, nullptr, this));
		setFrameStyle(QFrame::NoFrame);
		setAlignment(Qt::AlignLeft);
		setIndent(1);
		qApp->installEventFilter(this);
		setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, nullptr, this) / 255.0);
		setMouseTracking(true);
		setText(value);

		QFontMetrics fm{font()};
		QSize extra{1, 0};

		// Make it look good with the default ToolTip font on Mac, which has a small descent.
		if (fm.descent() == 2 && fm.ascent() >= 11) ++extra.rheight();

		setWordWrap(Qt::mightBeRichText(text()));
		QSize sh = sizeHint();
		move(pos);
		resize(sh + extra);
	}

	void ToolTipLabel::hideTip() {
		close();
		deleteLater();
	}

	bool ToolTipLabel::eventFilter(QObject*, QEvent*) { return false; }

	void ToolTipLabel::paintEvent(QPaintEvent* event) {
		QStylePainter p(this);
		QStyleOptionFrame opt;
		opt.initFrom(this);
		p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
		p.end();

		QLabel::paintEvent(event);
	}

	void ToolTipLabel::resizeEvent(QResizeEvent* event) {
		QStyleHintReturnMask frameMask;
		QStyleOption option;
		option.initFrom(this);
		if (style()->styleHint(QStyle::SH_ToolTip_Mask, &option, this, &frameMask)) {
			setMask(frameMask.region);  // GCOV_EXCL_LINE
		}

		QLabel::resizeEvent(event);
	}
}  // namespace quick_dra::gui
