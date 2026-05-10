// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QResizeEvent>
#include <app/controls/PageScrollArea.hpp>
#include <app/utils/LaidOut.hpp>
#include <app/utils/utils.hpp>
#include <map>
#include <optional>
#include <quick_dra/docs/xml.hpp>
#include <sstream>
#include <string>
#include <utility>

using namespace std::literals;

namespace quick_dra::gui {
	PageScrollArea::PageScrollArea(QWidget* parent) : QScrollArea{parent} {
		setFrameShape(QFrame::Shape::NoFrame);
		setFrameShadow(QFrame::Shadow::Plain);
		setLineWidth(0);
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
		setWidgetResizable(true);
	}

	std::pair<PageScrollArea*, QWidget*> PageScrollArea::setupPage(QWidget* page) {
		QHBoxLayout* rootLayout{};
		PageScrollArea* scrollArea{};
		QWidget* pageParent{};

		LaidOut{page}.createLayout(rootLayout, "", page,
		                           [](QLayout& layout) { layout.setContentsMargins(0, 0, 0, 0); });

		auto root = LaidOut{page, rootLayout};
		root.createWidget(scrollArea, "scrollArea");

		LaidOut<QWidget, QLayout>{nullptr}.createWidget(pageParent, "pageParent",
		                                                [&scrollArea = *scrollArea](QWidget& pageParent) {
			                                                scrollArea.setWidget(&pageParent);

			                                                pageParent.setSizePolicy(TakeWidth);
			                                                pageParent.setGeometry(0, 0, 10, 10);
		                                                });
		return {scrollArea, pageParent};
	}

	void PageScrollArea::resizeEvent(QResizeEvent* event) {
		QScrollArea::resizeEvent(event);
		auto const child = widget();
		auto const width = event->size().width();
		auto const height = child->sizePolicy().hasHeightForWidth() ? child->heightForWidth(width) : child->height();
		widget()->resize(width, height);
	}
}  // namespace quick_dra::gui
