// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QFormLayout>
#include <QPushButton>
#include <QTreeView>
#include <algorithm>
#include <app/controls/forms/field.hpp>
#include <app/utils/forms.hpp>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace quick_dra::gui {
	class ListViewBase : public QObject {
		Q_OBJECT;

	public:
		ListViewBase();

		void addToLayout(QWidget* parent, QFormLayout* layout, std::string_view label, QAbstractItemModel* model);

		bool isValid() const noexcept { return true; }

		void restyleField(bool lightMode);

	signals:
		void valueChanged();
		void validationChanged();

	protected:
	public:
		QFormLayout* parent{};
		QTreeView* view{};
		QPushButton* addButton{};
		QPushButton* removeButton{};
		bool attaching : 1 = false;
		bool dirty : 1 = false;
	};

	namespace detail {
		template <int Index, typename Variant>
		bool emplaceVariant(int index, Variant& variant) {
			if (index == Index) {
				variant.template emplace<Index>();
				return true;
			}
			return false;
		}
		template <int... Index, typename Variant>
		void emplaceVariantSomewhere(int index, Variant& variant, std::integer_sequence<int, Index...>) {
			(emplaceVariant<Index, Variant>(index, variant) || ...);
		}

		QVariant displayRole(year_month month);
		QVariant displayRole(ratio scale);
		QVariant displayRole(currency salary);

		QVariant editRole(year_month month);
		QVariant editRole(ratio scale);
		QVariant editRole(currency salary);

		bool readEditValue(year_month& month, std::string_view);
		bool readEditValue(ratio& scale, std::string_view);
		bool readEditValue(currency& salary, std::string_view);

		template <Declaration ColumnDecl>
		inline QVariant getData(typename ColumnDecl::target_type& item, int role) {
			switch (role) {
				case Qt::DisplayRole:
					return detail::displayRole(ColumnDecl::getField(item));
				case Qt::EditRole:
					return detail::editRole(ColumnDecl::getField(item));
				case Qt::TextAlignmentRole:
					return Qt::AlignRight;
				default:
					break;
			}
			return {};
		}

		template <Declaration ColumnDecl>
		inline auto lessThan(typename ColumnDecl::target_type const& lhs, typename ColumnDecl::target_type const& rhs) {
			return ColumnDecl::getField(const_cast<typename ColumnDecl::target_type&>(lhs)) <
			       ColumnDecl::getField(const_cast<typename ColumnDecl::target_type&>(rhs));
		}

		template <Declaration ColumnDecl>
		inline auto greaterThan(typename ColumnDecl::target_type const& lhs,
		                        typename ColumnDecl::target_type const& rhs) {
			return ColumnDecl::getField(const_cast<typename ColumnDecl::target_type&>(lhs)) >
			       ColumnDecl::getField(const_cast<typename ColumnDecl::target_type&>(rhs));
		}

		template <Declaration ColumnDecl>
		inline void stableSort(bool asc, std::vector<typename ColumnDecl::target_type>& items) {
			if (asc)
				std::stable_sort(items.begin(), items.end(), lessThan<ColumnDecl>);
			else
				std::stable_sort(items.begin(), items.end(), greaterThan<ColumnDecl>);
		}
	};  // namespace detail

	template <typename... T>
	struct Types {
		template <typename Cb, typename Fallback>
		    requires(std::same_as<decltype(std::declval<Fallback>()()),
		                          decltype(std::declval<Cb>()(std::declval<T>()))> &&
		             ...)
		static auto select(int index, Cb&& cb, Fallback&& fallback) -> decltype(auto) {
			static constexpr auto VariadicSize = static_cast<int>(sizeof...(T));
			if (index < VariadicSize) {
				std::variant<T...> variant{};
				emplaceVariantSomewhere(index, variant, std::make_integer_sequence<int, VariadicSize>{});
				return std::visit(std::forward<Cb>(cb), variant);
			}
			return fallback();
		}
	};

	class ListModelBase : public QAbstractItemModel {
		Q_OBJECT;

	public:
		using QAbstractItemModel::QAbstractItemModel;

		QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override {
			if (!this->hasIndex(row, column, parent) || parent.isValid()) {
				return {};
			}
			return this->createIndex(row, column);
		}

		QModelIndex parent(const QModelIndex&) const override { return {}; }

		int rowCount(const QModelIndex& parent = {}) const override {
			if (parent.isValid()) return 0;
			return static_cast<int>(dataRowCount());
		};

		int columnCount(const QModelIndex& parent = {}) const override {
			if (parent.isValid()) return 0;
			return static_cast<int>(dataColumnCount());
		};

		virtual size_t dataRowCount() const = 0;
		virtual size_t dataColumnCount() const = 0;

	signals:
		void valueChanged();
	};

	template <Declaration ItemsDecl, VectorItemDeclaration<ItemsDecl>... ItemDecls>
	class ListModel : public ListModelBase {
	public:
		using value_type = typename ItemsDecl::value_type;

		using ListModelBase::ListModelBase;

		void attach(ItemsDecl::target_type& target) {
			this->beginResetModel();
			ref_ = &ItemsDecl::getField(target);
			reorder();
			this->endResetModel();
		}

		size_t dataRowCount() const { return ref_ ? ref_->size() : 0; }
		size_t dataColumnCount() const override { return sizeof...(ItemDecls); }

		void addNewRow() {
			typename value_type::value_type newRow{};
			(setNewRowValue<ItemDecls>(newRow), ...);
			auto index = static_cast<int>(dataRowCount());
			this->beginInsertRows({}, index, index);
			if (ref_) ref_->push_back(std::move(newRow));
			this->endInsertRows();
		}

		template <typename Decl>
		static void setNewRowValue(typename value_type::value_type& newRow) {
			Decl::getField(newRow) = Decl::newRow();
		}

		void replaceRows(value_type&& rows) {
			this->beginResetModel();
			if (ref_) *ref_ = std::move(rows);
			this->endResetModel();
		}

		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
			auto& item = (*ref_)[static_cast<std::size_t>(index.row())];
			return selectColumn(
			    index.column(),
			    [&item, role]<Declaration ColumnDecl>(ColumnDecl) { return detail::getData<ColumnDecl>(item, role); },
			    []() -> QVariant { return {}; });
		}

		bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override {
			if (!index.isValid() || role != Qt::EditRole) return {};

			auto& item = (*ref_)[static_cast<std::size_t>(index.row())];
			auto const val = value.toString().toStdString();
			return selectColumn(
			    index.column(),
			    [&item, &val, self = this,
			     pos = static_cast<size_t>(index.column())]<Declaration ColumnDecl>(ColumnDecl) {
				    typename ColumnDecl::value_type local{};
				    if (detail::readEditValue(local, val)) {
					    if constexpr (ColumnDecl::unique) {
						    unsigned index = 0;
						    for (auto& row : *self->ref_) {
							    if (index == pos) {
								    ++index;
								    continue;
							    }

							    if (ColumnDecl::getField(row) == local) {
								    return false;
							    }

							    ++index;
						    }
					    }
					    ColumnDecl::getField(item) = std::move(local);
					    self->valueChanged();
					    return true;
				    }
				    return false;
			    },
			    []() { return false; });
		}

		Qt::ItemFlags flags(const QModelIndex& index) const override {
			auto defaultFlags = QAbstractItemModel::flags(index);
			if (index.isValid()) defaultFlags |= Qt::ItemIsEditable;
			return defaultFlags;
		}

		void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override {
			orderColumn_ = column;
			asc_ = (order == Qt::AscendingOrder);
			if (!ref_) return;
			reorder();
		}

		void reorder() {
			selectColumn(
			    orderColumn_,
			    [self = this, &ref = *ref_]<Declaration ColumnDecl>(ColumnDecl) {
				    self->layoutAboutToBeChanged();
				    detail::stableSort<ColumnDecl>(self->asc_, ref);
				    self->layoutChanged();
			    },
			    []() {});
		}

		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
			if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
				return {};
			}

			return selectColumn(
			    section,
			    []<Declaration ColumnDecl>(ColumnDecl) -> QVariant { return QString::fromUtf8(ColumnDecl::label); },
			    []() -> QVariant { return {}; });
		}

	private:
		value_type* ref_{};
		int orderColumn_{-1};
		bool asc_{true};

		template <typename Callback, typename Fallback>
		static auto selectColumn(int col, Callback&& cb, Fallback&& fb) -> decltype(auto) {
			return Types<ItemDecls...>::select(col, std::forward<Callback>(cb), std::forward<Fallback>(fb));
		}
	};

	template <Declaration ItemsDecl, VectorItemDeclaration<ItemsDecl>... ItemDecls>
	class ListView : public ListViewBase {
	public:
		using Model = ListModel<ItemsDecl, ItemDecls...>;
		using ListViewBase::addToLayout;
		void addToLayout(QWidget* parentWidget, QFormLayout* layout) {
			addToLayout(parentWidget, layout, ItemsDecl::label, new Model{this});
		}

		template <typename T>
		void connectTo(T* host) {
			auto model = static_cast<Model*>(this->view->model());
			QObject::connect(this, &ListViewBase::valueChanged, host, &T::updateCurrentValue);
			QObject::connect(this, &ListViewBase::validationChanged, host, &T::updateCurrentIsValid);
			QObject::connect(model, &Model::valueChanged, this, &ListViewBase::valueChanged);
		}

		void attach(typename ItemsDecl::target_type& target) {
			attaching = true;
			auto model = static_cast<Model*>(this->view->model());
			model->attach(target);
			attaching = false;
		}

		void readValue(typename ItemsDecl::target_type&) { dirty = false; }
	};
}  // namespace quick_dra::gui
