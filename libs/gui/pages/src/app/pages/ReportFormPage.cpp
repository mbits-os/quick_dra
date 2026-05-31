// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QLabel>
#include <QResizeEvent>
#include <app/pages/ReportFormPage.hpp>
#include <app/utils/LaidOut.hpp>
#include <app/utils/utils.hpp>
#include <format>
#include <map>
#include <optional>
#include <quick_dra/docs/xml.hpp>
#include <sstream>
#include <string>

using namespace std::literals;

namespace quick_dra::gui {
	namespace {
		static constexpr auto styleSheet = R"(
QFrame, QLabel {
	border: none;
}
QLabel {
    font-size: 9pt;
	padding: .25em;
	background-color: palette(base);
}

QLabel.groupHeader {
	font-weight: bold;
}

QLabel.followingHeader {
	padding-top: 1em;
}

QLabel.firstColumn {
	padding-left: 2em;
	padding-right: .5em;
}

QLabel.oddRow {
	background-color: palette(alternate-base);
}
)"sv;
	}  // namespace

	ReportFormPage::ReportFormPage(size_t index, QWidget* parent) : PagedWidget{parent}, index{index} {
		ui.setupUI(this);
	}
	ReportFormPage::~ReportFormPage() = default;

	void ReportFormPage::UI::setupUI(QWidget* ReportFormPage) {
		auto const [scrollArea, pageParent] = PageScrollArea::setupPage(ReportFormPage);
		gridParent = pageParent;
		gridParent->setStyleSheet(QString::fromUtf8(gui::styleSheet));

		LaidOut{gridParent}.createLayout(gridLayout, "gridLayout", gridParent, [](QGridLayout& layout) {
			layout.setContentsMargins(0, 0, 0, 0);
			layout.setSpacing(0);
		});
	}

	QString toolTip(std::string_view description) {
		auto const split = split_sv(description, '\n'_sep, 1);
		auto fragment = xml{{}, {}}.with(E("div"sv, {{"style", "white-space:pre"}}).with(E("b"sv).with(split[0])));

		if (split.size() > 1) {
			for (auto const& line : split_sv(split[1], '\n'_sep)) {
				fragment.with(
				    E("div"sv, {{"style", "white-space:pre"}}).with(E("i"sv).with(std::format("\xC2\xB7 {}", line))));
			}
		}

		std::ostringstream oss;
		oss << fragment;
		return QString::fromUtf8(oss.str());
	}

	void ReportFormPage::connectPage() {
		auto const formatted = globals().data().formatReport(index);
		setWindowTitle(QString::fromUtf8(formatted.title));

		int rowNumber{-1};

		for (auto const& key : formatted.order) {
			auto it = formatted.data.find(key);
			if (it == formatted.data.end()) {
				continue;
			}

			bool firstGroup = rowNumber == -1;

			{
				QLabel* label;
				auto const row = LaidOutGrid{ui.gridParent, ui.gridLayout, ++rowNumber};
				row.createWidget(
				    label, "", {.column = 0, .colSpan = 3},
				    [&dataSection = it->second, firstGroup, even = (rowNumber % 2 == 0)](QLabel& label) {
					    label.setAlignment(Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignBaseline);
					    label.setText(QString::fromUtf8(std::format("{}. {}", dataSection.name, dataSection.label)));
					    label.setProperty("class", QStringList{
					                                   "groupHeader",
					                                   firstGroup ? "firstHeader" : "followingHeader",
					                                   even ? "evenRow" : "oddRow",
					                               });
					    label.setFrameStyle(QFrame::StyledPanel);
					    label.setWordWrap(true);
					    label.setToolTip(gui::toolTip(dataSection.label));
					    label.setSizePolicy(TakeAll);
				    });
			}

			for (auto const& field : it->second.fields) {
				QLabel* indexCol;
				QLabel* valueCol;
				QLabel* fillCol;
				auto const row = LaidOutGrid{ui.gridParent, ui.gridLayout, ++rowNumber};
				QString toolTip = gui::toolTip(field.label);
				row.createWidget(
				       indexCol, "", {.column = 0},
				       [text = QString::number(field.number), &toolTip, even = (rowNumber % 2 == 0)](QLabel& label) {
					       label.setText(text);
					       label.setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignBaseline);
					       label.setProperty("class", QStringList{
					                                      "firstColumn",
					                                      even ? "evenRow" : "oddRow",
					                                  });
					       label.setFrameStyle(QFrame::StyledPanel);
					       label.setToolTip(toolTip);
					       label.setSizePolicy(TakeAll);
				       })
				    .createWidget(
				        valueCol, "", {.column = 1},
				        [text = QString::fromUtf8(field.formatted), alignement = field.alignement, &toolTip,
				         even = (rowNumber % 2 == 0)](QLabel& label) {
					        label.setText(text);
					        label.setProperty("class", QStringList{
					                                       "secondColumn",
					                                       even ? "evenRow" : "oddRow",
					                                   });
					        label.setFrameStyle(QFrame::StyledPanel);
					        label.setAlignment(Qt::AlignmentFlag::AlignBaseline |
					                           (alignement == alignement::right ? Qt::AlignmentFlag::AlignRight
					                                                            : Qt::AlignmentFlag::AlignLeft));
					        label.setFrameStyle(QFrame::StyledPanel);
					        label.setToolTip(toolTip);
					        label.setSizePolicy(TakeAll);
				        })
				    .createWidget(fillCol, "", {.column = 2}, [&toolTip, even = (rowNumber % 2 == 0)](QLabel& label) {
					    label.setText("");
					    label.setProperty("class", QStringList{
					                                   "thirdColumn",
					                                   even ? "evenRow" : "oddRow",
					                               });
					    label.setFrameStyle(QFrame::StyledPanel);
					    label.setFrameStyle(QFrame::StyledPanel);
					    label.setToolTip(toolTip);
					    label.setSizePolicy(TakeAll / 1_XStretch);
				    });
			}
		}
	}

	bool ReportFormPage::event(QEvent* event) {
		switch (event->type()) {
			case QEvent::ApplicationPaletteChange:
				[[fallthrough]];
			case QEvent::PaletteChange:
				ui.gridParent->setStyleSheet(ui.gridParent->styleSheet());
				break;

			default:
				break;
		}
		return PagedWidget::event(event);
	}
}  // namespace quick_dra::gui
