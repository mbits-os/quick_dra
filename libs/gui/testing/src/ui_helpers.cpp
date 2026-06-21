// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include "ui_helpers.hpp"
#include <QLabel>
#include <QLayout>
#include <format>
#include <fstream>
#include <quick_dra/base/str.hpp>
#include <sstream>
#include <string>
#include <utility>

#ifdef _WIN32
#define NOMINMAX
#include <io.h>
#endif

using std::literals::operator""sv;

#ifdef WIN32
inline char* mkdtemp(char* buffer) {
	_mktemp(buffer);
	auto const path = std::filesystem::path{quick_dra::as_u8v(std::string_view{buffer})};
	create_directories(path);
	return buffer;
}
#endif

void itemDataRow(QAbstractItemModel* model, DataRow& result, int row, int columns, int role) {
	for (auto column = 0; column < columns; ++column) {
		auto const index = model->index(row, column);
		result.push_back(model->data(index, role));
	}
}

DataSet itemData(QAbstractItemModel* model, int role) {
	DataSet result{};
	auto const rows = model->rowCount();
	auto const columns = model->columnCount();

	result.reserve(rows);
	for (auto row = 0; row < rows; ++row) {
		result.emplace_back();
		result.back().reserve(columns);
		itemDataRow(model, result.back(), row, columns, role);
	}

	return result;
}

std::filesystem::path TmpDir::make_temp_dir() {
	auto const path = std::filesystem::temp_directory_path() / "dirXXXXXX"sv;
	auto name = quick_dra::as_str(path.u8string());
	return quick_dra::as_u8v(std::string_view{mkdtemp(name.data())});
}

std::string fileContents(std::filesystem::path const& path) {
	auto file = std::ifstream{path};
	auto str = std::ostringstream{};
	str << file.rdbuf();
	return std::move(str).str();
}

void setFileContents(std::filesystem::path const& path, std::string_view contents) {
	auto file = std::ofstream{path};
	file.write(contents.data(), contents.size());
}

void writeConfig(std::filesystem::path const& cwd, std::filesystem::path const& data) {
	setFileContents(cwd / ".quick_dra.yaml"sv, fileContents(data / ".quick_dra.testing.yaml"sv));
	auto out_file = std::ofstream{cwd / "config.ini"sv, std::ios::binary | std::ios::out};
	static constexpr auto ini = "[Settings]\nReportIndex=1\nYearMonth=2020/5\n"sv;
	out_file.write(ini.data(), ini.size());
}

QSettings openSettings(std::filesystem::path const& cwd) {
	return {QString::fromUtf8(quick_dra::as_sv((cwd / "config.ini").generic_u8string())), QSettings::IniFormat};
}

void enumObject(QObject const* obj, size_t indent) {
	std::string const indent_s(indent, ' ');
	for (auto const* child : obj->children()) {
		auto const label = qobject_cast<QLabel const*>(child);
		auto const layout = qobject_cast<QLayout const*>(child);

#define TYPE(PTR) PTR->metaObject()->className() << static_cast<void const*>(PTR)
		if (label) {
			qDebug() << indent_s.c_str() << TYPE(child) << label->text();
		} else if (layout) {
			auto const margins = layout->contentsMargins();
			qDebug() << indent_s.c_str() << TYPE(child) << child->objectName()
			         << std::format("({}, {}, {}, {}), {}", margins.left(), margins.top(), margins.right(),
			                        margins.bottom(), layout->spacing())
			                .c_str();
		} else {
			qDebug() << indent_s.c_str() << TYPE(child) << child->objectName();
		}

		if (layout) {
			auto const count = layout->count();
			for (auto index = 0; index < count; ++index) {
				auto const item = layout->itemAt(index);
				QObject const* layoutItem = item->widget();
				if (!layoutItem) layoutItem = item->layout();
				auto const spacer = item->spacerItem();
				if (layoutItem) {
					qDebug() << indent_s.c_str() << " -" << TYPE(layoutItem);
				} else if (spacer) {
					qDebug() << indent_s.c_str() << " - <spacer>";
				} else {
					qDebug() << indent_s.c_str() << " - <null>";
				}
			}
		}
		enumObject(child, indent + 2);
	}
}

std::optional<QList<QVariant>> takeFirst(QSignalSpy& spy) {
	if (spy.empty()) {
		spy.wait();
	}
	if (spy.empty()) return std::nullopt;
	return spy.takeFirst();
}
