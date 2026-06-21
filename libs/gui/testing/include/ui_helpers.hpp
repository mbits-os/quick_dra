// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QAbstractItemModel>
#include <QList>
#include <QSettings>
#include <QSignalSpy>
#include <QString>
#include <QVariant>
#include <QWidget>
#include <filesystem>
#include <string>

struct ParentContext {
	explicit ParentContext(QObject* object) : ref{object} { curr() = this; }
	explicit ParentContext(QObject& object) : ref{&object} { curr() = this; }
	~ParentContext() { curr() = prev; }

	template <typename T>
	static inline T locateChild(QAnyStringView aName, Qt::FindChildOptions options = Qt::FindChildrenRecursively) {
		return curr()->ref->findChild<T>(aName, options);
	}

private:
	static ParentContext*& curr() {
		static thread_local ParentContext* on_thread{nullptr};
		return on_thread;
	}
	QObject* ref{};
	ParentContext* prev{curr()};
};

#define JOIN2(X, Y) X##Y
#define JOIN1(X, Y) JOIN2(X, Y)
#define PARENT_CONTEXT(REF) \
	auto const JOIN1(parentContext, __LINE__) = ParentContext { REF }

#define ENSURE_CHILD(TYPE, NAME)                                \
	auto const NAME = ParentContext::locateChild<TYPE*>(#NAME); \
	QVERIFY(NAME)

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
void setFileContents(std::filesystem::path const& path, std::string_view contents);
void writeConfig(std::filesystem::path const& cwd, std::filesystem::path const& data);
QSettings openSettings(std::filesystem::path const& cwd);
void enumObject(QObject const* obj, size_t indent = 0);
std::optional<QList<QVariant>> takeFirst(QSignalSpy& spy);
template <std::same_as<QSignalSpy>... Spy>
static inline void waitFor(Spy&... spy) {
	(takeFirst(spy), ...);
}
