// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once
#include <QFormLayout>
#include <QLayout>
#include <QString>
#include <QToolButton>
#include <memory>
#include <string_view>
#include <utility>

namespace quick_dra::gui {
	template <typename T>
	struct EmptyCallback {
		void operator()(T const&) {}
	};

	template <typename QType, typename QLayoutType>
	class LaidOut;
	template <typename QType>
	class LaidOutForm;
	template <typename QType>
	class LaidOutGrid;

	template <typename QType, typename QLayoutType>
	class LaidOut {
	public:
		LaidOut(QType* self, QLayoutType* layout) : self_{self}, layout_{layout} {}
		LaidOut(QType* self) : LaidOut{self, nullptr} {}

		template <typename QLayoutType2>
		LaidOut<QType, QLayoutType2> withLayout(QLayoutType2* layout) const {
			return {self_, layout};
		}

		template <typename T, typename Cb = EmptyCallback<T>>
		inline T* addWidget(QAnyStringView name, Cb&& cb = {}) const {
			auto result = std::make_unique<T>(self_);
			result->setObjectName(name);
			cb(*result.get());
			if (layout_) layout_->addWidget(result.get());
			return result.release();
		}

		template <typename T, typename Cb = EmptyCallback<T>>
		inline LaidOut const& createWidget(T*& out, QAnyStringView name, Cb&& cb = {}) const {
			out = addWidget<T>(name, std::forward<Cb>(cb));
			return *this;
		}

		template <typename T, typename Cb = EmptyCallback<T>>
		inline T* addLayout(QAnyStringView name, QWidget* layoutParent, Cb&& cb = {}) const {
			auto result = std::make_unique<T>(layoutParent);
			result->setObjectName(name);
			cb(*result.get());
			if constexpr (!std::same_as<QLayoutType, QLayout>) {
				if (layout_) layout_->addLayout(result.get());
			}
			return result.release();
		}

		template <typename T, typename Cb = EmptyCallback<T>>
		    requires(!std::convertible_to<Cb, QWidget*>)
		inline T* addLayout(QAnyStringView name, Cb&& cb = {}) const {
			return addLayout<T>(name, nullptr, std::forward<Cb>(cb));
		}

		template <typename T, std::invocable<T&> Cb = EmptyCallback<T>>
		inline LaidOut const& createLayout(T*& out,
		                                   QAnyStringView name,
		                                   QWidget* layoutParent = nullptr,
		                                   Cb&& cb = {}) const {
			out = addLayout<T>(name, layoutParent, std::forward<Cb>(cb));
			return *this;
		}

		template <typename T, std::invocable<T&> Cb>
		inline LaidOut const& createLayout(T*& out, QAnyStringView name, Cb&& cb) const {
			return createLayout(out, name, nullptr, std::forward<Cb>(cb));
		}

	private:
		QType* self_;
		QLayoutType* layout_;
	};

	template <typename QType>
	class LaidOutForm {
	public:
		LaidOutForm(QType* self, QFormLayout* layout, int row) : self_{self}, layout_{layout}, row_{row} {}

		template <typename QLayoutType2>
		LaidOut<QType, QLayoutType2> withLayout(QLayoutType2* layout) const {
			return {self_, layout};
		}

		template <typename T, typename Cb = EmptyCallback<T>>
		inline T* setWidget(QAnyStringView name, QFormLayout::ItemRole role, Cb&& cb = {}) const {
			auto result = std::make_unique<T>(self_);
			result->setObjectName(name);
			cb(*result.get());
			layout_->setWidget(row_, role, result.get());
			return result.release();
		}

		template <typename T, typename Cb = EmptyCallback<T>>
		inline LaidOutForm const& createWidget(T*& out,
		                                       QAnyStringView name,
		                                       QFormLayout::ItemRole role,
		                                       Cb&& cb = {}) const {
			out = setWidget<T>(name, role, std::forward<Cb>(cb));
			return *this;
		}

		template <typename T, typename Cb = EmptyCallback<T>>
		inline T* addLayout(QAnyStringView name, QFormLayout::ItemRole role, Cb&& cb = {}) const {
			auto result = std::make_unique<T>(nullptr);
			result->setObjectName(name);
			cb(*result.get());
			if (layout_) layout_->setLayout(row_, role, result.get());
			return result.release();
		}

		template <typename T, typename Cb = EmptyCallback<T>>
		inline LaidOutForm const& createLayout(T*& out,
		                                       QAnyStringView name,
		                                       QFormLayout::ItemRole role,
		                                       Cb&& cb = {}) const {
			out = addLayout<T>(name, role, std::forward<Cb>(cb));
			return *this;
		}

	private:
		QType* self_;
		QFormLayout* layout_;
		int row_;
	};

	template <typename QType>
	class LaidOutGrid {
	public:
		struct GridStats {
			int column{0};
			int rowSpan{1};
			int colSpan{1};
		};
		LaidOutGrid(QType* self, QGridLayout* layout, int row) : self_{self}, layout_{layout}, row_{row} {}

		template <typename QLayoutType2>
		LaidOut<QType, QLayoutType2> withLayout(QLayoutType2* layout) const {
			return {self_, layout};
		}

		template <typename T, typename Cb = EmptyCallback<T>>
		inline T* addWidget(QAnyStringView name, GridStats const& stats, Cb&& cb = {}) const {
			auto result = std::make_unique<T>(self_);
			result->setObjectName(name);
			cb(*result.get());
			layout_->addWidget(result.get(), row_, stats.column, stats.rowSpan, stats.colSpan);
			return result.release();
		}

		template <typename T, typename Cb = EmptyCallback<T>>
		inline LaidOutGrid const& createWidget(T*& out,
		                                       QAnyStringView name,
		                                       GridStats const& stats,
		                                       Cb&& cb = {}) const {
			out = addWidget<T>(name, stats, std::forward<Cb>(cb));
			return *this;
		}

		template <typename T, typename Cb = EmptyCallback<T>>
		inline T* addLayout(QAnyStringView name, GridStats const& stats, Cb&& cb = {}) const {
			auto result = std::make_unique<T>(nullptr);
			result->setObjectName(name);
			cb(*result.get());
			if (layout_) layout_->addLayout(result.get(), row_, stats.column, stats.rowSpan, stats.colSpan);
			return result.release();
		}

		template <typename T, typename Cb = EmptyCallback<T>>
		inline LaidOutGrid const& createLayout(T*& out,
		                                       QAnyStringView name,
		                                       GridStats const& stats,
		                                       Cb&& cb = {}) const {
			out = addLayout<T>(name, stats, std::forward<Cb>(cb));
			return *this;
		}

	private:
		QType* self_;
		QGridLayout* layout_;
		int row_;
	};

	template <typename QParent, typename QLayoutType>
	LaidOut(QParent*, QLayoutType*) -> LaidOut<QParent, QLayoutType>;

	template <typename QParent>
	LaidOut(QParent*) -> LaidOut<QParent, QLayout>;

	template <typename QParent>
	LaidOutForm(QParent*, QFormLayout*, int) -> LaidOutForm<QParent>;

	template <typename QParent>
	LaidOutGrid(QParent*, QGridLayout*, int) -> LaidOutGrid<QParent>;
}  // namespace quick_dra::gui
