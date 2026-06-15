// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QAbstractItemModel>
#include <QList>
#include <QSettings>
#include <QString>
#include <QVariant>
#include <QWidget>
#include <filesystem>
#include <string>

template <typename T>
inline T locateChild(QObject* root, QAnyStringView aName, Qt::FindChildOptions options = Qt::FindChildrenRecursively) {
	return root->findChild<T>(aName, options);
}

template <typename T>
inline T locateChild(QObject& root, QAnyStringView aName, Qt::FindChildOptions options = Qt::FindChildrenRecursively) {
	return root.findChild<T>(aName, options);
}

using DataRow = QList<QVariant>;
using DataSet = QList<DataRow>;

DataSet itemData(QAbstractItemModel* model, int role);

struct TmpDir {
	TmpDir() { std::filesystem::current_path(dirname_); }
	TmpDir(TmpDir const&) = delete;
	TmpDir(TmpDir&&) = delete;
	~TmpDir() {
		std::error_code ec{};
		std::filesystem::current_path(cwd_);
		std::filesystem::remove_all(dirname_, ec);
	}

	[[nodiscard]] std::filesystem::path const& cwd() const noexcept { return dirname_; };

private:
	static std::filesystem::path make_temp_dir();

	std::filesystem::path dirname_{make_temp_dir()};
	std::filesystem::path cwd_{std::filesystem::current_path()};
};

std::string fileContents(std::filesystem::path const& path);
inline QString qFileContents(std::filesystem::path const& path) { return QString::fromUtf8(fileContents(path)); }
void writeConfig(std::filesystem::path const& cwd, std::filesystem::path const& data);
QSettings openSettings(std::filesystem::path const& cwd);
void enumObject(QObject const* obj, size_t indent = 0);
