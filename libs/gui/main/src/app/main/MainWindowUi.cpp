// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QPushButton>
#include <app/main/MainWindow.hpp>
#include <app/utils/LaidOut.hpp>
#include <app/utils/utils.hpp>
#include <quick_dra/version.hpp>

using namespace std::literals;

namespace quick_dra::gui {
	namespace {
		auto withCssMargins(int top = 0, int right = -1, int bottom = -1, int left = -1) {
			if (right < 0) right = top;
			if (bottom < 0) bottom = top;
			if (left < 0) left = right;
			return [top, right, bottom, left](QLayout& layout) { layout.setContentsMargins(left, top, right, bottom); };
		}
	}  // namespace

	void MainWindow::setupUi() {
		if (objectName().isEmpty()) setObjectName("MainWindow");
		restorePosition();
		LaidOut{this}.createWidget(centralWidget, "centralWidget");

		HeaderShadow* shadow{};
		auto root = LaidOut{centralWidget};
		root.createLayout(verticalLayout, "verticalLayout", centralWidget,
		                  [](QVBoxLayout& layout) {
			                  layout.setContentsMargins(0, 0, 0, 0);
			                  layout.setSpacing(0);
		                  })
		    .createWidget(shadow, "", [](auto& shadow) {
			    shadow.setShadowHeight(12);
			    shadow.setShadowForce(.25f);
		    });

		QHBoxLayout* messageLayout{};
		QLabel* messageLabel{};
		QPushButton* reloadButton{};

		root.withLayout(verticalLayout)
		    .createWidget(pageHeader, "pageHeader")
		    .createWidget(stackedWidget, "stackedWidget")
		    .createWidget(messageBar, "messageBar", [](auto& wgt) {
			    wgt.setSizePolicy(TakeWidth);
			    wgt.setVisible(false);
		    });

		LaidOut{messageBar}
		    .createLayout(messageLayout, "messageLayout", messageBar)
		    .withLayout(messageLayout)
		    .createWidget(messageLabel, "messageLabel",
		                  [](QLabel& label) {
			                  label.setText("Konfiguracja Quick-DRA została zmodyfikowana");
			                  label.setSizePolicy(TakeWidth / 1_XStretch);
		                  })
		    .createWidget(reloadButton, "reloadButton", [](QPushButton& button) {
			    button.setText("Odśwież");
			    button.setSizePolicy(TakeWidth);
		    });

		setCentralWidget(centralWidget);
		shadow->raise();

		QObject::connect(pageHeader, &PageHeader::moved, shadow, &HeaderShadow::targetMoved);

		pageStack = new PageStack(pageHeader, stackedWidget);
		pageStack->setObjectName("pageStack");

		QObject::connect(pageStack, &PageStack::titleChanged, this, &MainWindow::updateTitle);
		QObject::connect(reloadButton, &QPushButton::clicked, this, &MainWindow::reloadConfig);

		updateStyles();
	}

	void MainWindow::updateStyles() {
		auto const pal = palette();
		auto const lightMode =
		    pal.color(QPalette::Window).toHsl().lightness() >= pal.color(QPalette::WindowText).toHsl().lightness();

		messageBar->setStyleSheet(lightMode ? "#messageBar {background-color: rgba(231, 204, 70, 0.8)}"
		                                    : "#messageBar {background-color: rgba(110, 72, 8, 0.3)}");
	}
}  // namespace quick_dra::gui
