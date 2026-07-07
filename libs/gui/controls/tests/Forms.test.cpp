// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QTest>
#include <app/controls/Forms.hpp>
#include <app/gui/PagedWidget.hpp>
#include <quick_dra/base/str.hpp>
#include "ControlsTest.hpp"
#include "palette_override.hpp"

using namespace quick_dra::gui;
using namespace quick_dra;

#define NBSP "\u00A0"
#ifdef WIN32
#define GROUP_SEP NBSP
#else
#define GROUP_SEP "\u202F"
#endif

namespace quick_dra::gui {

	struct Data {
		std::string field_1{};
		std::string field_2{};
		insurance_title field_3{};
		std::string field_4{};
	};

	struct SomeCharsValidator : detail::NonEmptyValidator {
		static size_t max_length() noexcept { return 10; }
	};

	DECLARE_FIELD(Field1Declaration, detail::NonEmptyValidator, Data, field_1) {
		static constexpr auto id = "field1"sv;
		static constexpr auto label = "Field #1"sv;
		static constexpr auto error_message = ""sv;
	};

	DECLARE_FIELD(Field2Declaration, SomeCharsValidator, Data, field_2) {
		static constexpr auto id = "field2"sv;
		static constexpr auto label = "Field #2"sv;
		static constexpr auto error_message = ""sv;
	};

	DECLARE_FIELD(Field3Declaration, detail::InsuranceTitleValidator, Data, field_3) {
		static constexpr auto id = "field3"sv;
		static constexpr auto label = "Field #3"sv;
		static constexpr auto error_message = TitleDeclaration::error_message;
	};

	DECLARE_FIELD_HEADER(KeyAEnumDeclaration, id_card_validator, Data, field_1) {
		static constexpr auto id = "keyA"sv;
		ENUM_KEY(A);
		ERROR_CHECKSUM_NEEDED("Klucz A");
	};

	DECLARE_FIELD_HEADER(KeyBEnumDeclaration, pl_passport_validator, Data, field_2) {
		static constexpr auto id = "keyB"sv;
		ENUM_KEY(B);
		ERROR_CHECKSUM_NEEDED("Klucz B");
	};

	DECLARE_FIELD(KeyCEnumDeclaration, social_id_validator, Data, field_4) {
		static constexpr auto id = "keyC"sv;
		ENUM_KEY(C);
		ERROR_CHECKSUM_NEEDED("Klucz C");
	};

	using DocumentABCCombo = DocumentComboBox<KeyAEnumDeclaration, KeyBEnumDeclaration, KeyCEnumDeclaration>;

	using LineEditUI = FormUI<LineEdit<Field1Declaration>, LineEdit<Field2Declaration>, LineEdit<Field3Declaration>>;

	using DocumentUI = FormUI<DocumentABCCombo>;

	using HistoryListView = ListView<EmploymentHistoryDeclaration,
	                                 EmploymentHistorySinceDeclaration,
	                                 EmploymentHistoryPartTimeScaleDeclaration,
	                                 EmploymentHistorySalaryDeclaration>;
	using HistoryUI = FormUI<HistoryListView>;

	struct TestableModel : HistoryListView::Model {
		using HistoryListView::Model::Model;
		QModelIndex makeInvalidIndex(int row, int column) { return this->createIndex(row, column); }
	};

	struct TestPage : PagedWidget {
		Q_OBJECT

	public slots:
		void updateCurrentValue() {}
		void updateFormValid() {}
	};

	QStringList itemTexts(QComboBox const& combo) {
		QStringList result{};
		result.reserve(combo.count());
		auto const count = combo.count();
		for (auto index = 0; index < count; ++index) {
			result.push_back(combo.itemText(index));
		}
		return result;
	}

	QStringList itemData(QComboBox const& combo) {
		QStringList result{};
		result.reserve(combo.count());
		auto const count = combo.count();
		for (auto index = 0; index < count; ++index) {
			result.push_back(combo.itemData(index).toString());
		}
		return result;
	}

	using Row = QList<QVariant>;
	using DataSet = QList<Row>;

	void itemDataRow(QAbstractItemModel* model, Row& result, int row, int columns, int role) {
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
}  // namespace quick_dra::gui

#define QCOMPARE_STR(lhs, rhs)                                                                               \
	do {                                                                                                     \
		QString s1{lhs};                                                                                     \
		QString s2{rhs};                                                                                     \
		if (!QTest::qCompareOp<QTest::ComparisonOperation::Equal>(s1, s2, #lhs, #rhs, __FILE__, __LINE__)) { \
			QTEST_FAIL_ACTION;                                                                               \
		}                                                                                                    \
	} while (false)

#define QCOMPARE_ACCESS(type, var, data) QCOMPARE_EQ(type::getField(var), data)
#define QCOMPARE_PROP_ACCESS(type, var, prop, data) QCOMPARE_EQ(type::getField(var).prop, data)

#define QVERIFY_LINE_EDIT(id, label, data)                                                      \
	do {                                                                                        \
		auto lineEditWidget = page.findChild<QLineEdit*>(id "Edit");                            \
		QVERIFY2(lineEditWidget, "No QLineEdit named " id "Edit");                              \
		auto labelWidget = qobject_cast<QLabel*>(ui.formLayout->labelForField(lineEditWidget)); \
		QVERIFY(labelWidget);                                                                   \
		QCOMPARE_STR(labelWidget->text(), label);                                               \
		QCOMPARE_STR(lineEditWidget->text(), data);                                             \
	} while (false)

#define QVERIFY_COMBO_BOX(id, label, currIndex, ...)                                            \
	do {                                                                                        \
		auto comboBoxWidget = page.findChild<QComboBox*>(id "ComboBox");                        \
		QVERIFY2(comboBoxWidget, "No QComboBox named " id "ComboBox");                          \
		auto labelWidget = qobject_cast<QLabel*>(ui.formLayout->labelForField(comboBoxWidget)); \
		QVERIFY(labelWidget);                                                                   \
		QStringList items{__VA_ARGS__};                                                         \
		QCOMPARE_STR(labelWidget->text(), label);                                               \
		QCOMPARE_EQ(itemTexts(*comboBoxWidget), items);                                         \
		QCOMPARE_EQ(comboBoxWidget->currentIndex(), currIndex);                                 \
	} while (false)

#define QVERIFY_HAS_ERROR_ROW(id)                                                       \
	do {                                                                                \
		auto const error = page.findChild<QLayout*>(id "Error");                        \
		auto const errorLabel = page.findChild<QLabel*>(id "ErrorLabel");               \
		QVERIFY2(error, "No QLayout named " id "Error");                                \
		QVERIFY2(errorLabel, "No QLabel named " id "ErrorLabel");                       \
		QVERIFY2(!ui.formLayout->isRowVisible(error), id "Error row should be hidden"); \
		QVERIFY(ui.isValid());                                                          \
	} while (false)

#define QVERIFY_IS_INVALID(id, message)                                                 \
	do {                                                                                \
		auto const error = page.findChild<QLayout*>(id "Error");                        \
		auto const errorLabel = page.findChild<QLabel*>(id "ErrorLabel");               \
		QVERIFY2(ui.formLayout->isRowVisible(error), id "Error row should be visible"); \
		QVERIFY(!ui.isValid());                                                         \
		QCOMPARE_STR(errorLabel->text(), message);                                      \
	} while (false)

#define QVERIFY_IS_VALID(id)                                                            \
	do {                                                                                \
		auto const error = page.findChild<QLayout*>(id "Error");                        \
		QVERIFY2(!ui.formLayout->isRowVisible(error), id "Error row should be hidden"); \
		QVERIFY(ui.isValid());                                                          \
	} while (false)

#define SET_TEXT_(id, data, type, suffix)              \
	do {                                               \
		auto child = page.findChild<type*>(id suffix); \
		child->setText(data);                          \
	} while (false)

#define SET_EDIT_TEXT(id, data) SET_TEXT_(id, data, QLineEdit, "Edit")

#define SELECT_OPTION(id, curIndex)                             \
	do {                                                        \
		auto child = page.findChild<QComboBox*>(id "ComboBox"); \
		child->setCurrentIndex(curIndex);                       \
	} while (false)

#define QVERIFY_TEXT_IS_VALID_(id, data, type, suffix) \
	do {                                               \
		SET_TEXT_(id, data, type, suffix);             \
		QVERIFY_IS_VALID(id);                          \
	} while (false)

#define QVERIFY_EDIT_TEXT_IS_VALID(id, data) QVERIFY_TEXT_IS_VALID_(id, data, QLineEdit, "Edit")

#define QVERIFY_IS_VALID_WITH_TEXT(id, data)                         \
	do {                                                             \
		auto lineEditWidget = page.findChild<QLineEdit*>(id "Edit"); \
		QCOMPARE_STR(lineEditWidget->text(), data);                  \
		QVERIFY_IS_VALID(id);                                        \
	} while (false)

#define QVERIFY_TEXT_IS_INVALID_(id, data, message, type, suffix) \
	do {                                                          \
		SET_TEXT_(id, data, type, suffix);                        \
		QVERIFY_IS_INVALID(id, message);                          \
	} while (false)

#define QVERIFY_EDIT_TEXT_IS_INVALID(id, data, message) QVERIFY_TEXT_IS_INVALID_(id, data, message, QLineEdit, "Edit")

#define QVERIFY_IS_INVALID_WITH_TEXT(id, data, message)              \
	do {                                                             \
		auto lineEditWidget = page.findChild<QLineEdit*>(id "Edit"); \
		QCOMPARE_STR(lineEditWidget->text(), data);                  \
		QVERIFY_IS_INVALID(id, message);                             \
	} while (false)

void ControlsTest::Forms_access() {
	quick_dra::person person{
	    .last_name = "Last"s,
	    .id_card = "id-card"s,
	    .passport = "passport"s,
	    .first_name = "First"s,
	    .kind = "document-kind"s,
	    .document = "a-number-in-document",
	};

	quick_dra::payer_t payer{person, "tax-id"s, "payer-social-id"s};
	gui::insured_type insured{
	    person,
	    {.title_code = "code", .pension_right = 1, .disability_level = 2},
	    "insured-social-id"s,
	    {gui::insured_type::history_type{
	        .since = 2024y / August,
	        .part_time_scale = {7, 12},
	        .salary = 10'123.45_PLN,
	    }},
	};
	auto& history = insured.history.front();

	QCOMPARE_ACCESS(FirstNameDeclaration, person, "First"sv);
	QCOMPARE_ACCESS(LastNameDeclaration, person, "Last"sv);
	QCOMPARE_ACCESS(IdCardEnumDeclaration, person, "id-card"sv);
	QCOMPARE_ACCESS(PassportEnumDeclaration, person, "passport"sv);

	QCOMPARE_ACCESS(TaxIdDeclaration, payer, "tax-id"sv);
	QCOMPARE_ACCESS(SocialIdDeclaration, payer, "payer-social-id"s);

	QCOMPARE_PROP_ACCESS(TitleDeclaration, insured, title_code, "code"sv);
	QCOMPARE_PROP_ACCESS(TitleDeclaration, insured, pension_right, 1);
	QCOMPARE_PROP_ACCESS(TitleDeclaration, insured, disability_level, 2);
	QCOMPARE_ACCESS(SocialIdEnumDeclaration, insured, "insured-social-id"sv);

	QCOMPARE_ACCESS(EmploymentHistoryDeclaration, insured,
	                (std::vector{
	                    gui::insured_type::history_type{
	                        .since = 2024y / August,
	                        .part_time_scale = {7, 12},
	                        .salary = 10'123.45_PLN,
	                    },
	                }));
	QCOMPARE_ACCESS(EmploymentHistorySinceDeclaration, history, 2024y / August);
	QCOMPARE_PROP_ACCESS(EmploymentHistoryPartTimeScaleDeclaration, history, num, 7);
	QCOMPARE_PROP_ACCESS(EmploymentHistoryPartTimeScaleDeclaration, history, den, 12);
	QCOMPARE_ACCESS(EmploymentHistorySalaryDeclaration, history, 10'123.45_PLN);

	auto const today = get_today();
	auto const nextSince = today.year() / today.month() + months{1};

	QCOMPARE_EQ(EmploymentHistorySinceDeclaration::newRow(), nextSince);
	QCOMPARE_EQ(EmploymentHistoryPartTimeScaleDeclaration::newRow().num, 1);
	QCOMPARE_EQ(EmploymentHistoryPartTimeScaleDeclaration::newRow().den, 1);
	QCOMPARE_EQ(EmploymentHistorySalaryDeclaration::newRow(), minimal_salary);
}

void ControlsTest::Forms_lineEdit() {
	TestPage page{};
	LineEditUI ui{};
	ui.setupPageUI(&page);
	Data data{.field_1 = "data1"s, .field_2 = "data2"s, .field_3 = {"9999"s, 9, 9}};
	ui.attach(data);

	QVERIFY_HAS_ERROR_ROW("field1");
	QVERIFY_HAS_ERROR_ROW("field2");
	QVERIFY_HAS_ERROR_ROW("field3");
	QVERIFY_LINE_EDIT("field1", "Field #1", "data1");
	QVERIFY_LINE_EDIT("field2", "Field #2", "data2");
	QVERIFY_LINE_EDIT("field3", "Field #3", "9999 9 9");

	SET_EDIT_TEXT("field1", "something else");

	QVERIFY_EDIT_TEXT_IS_INVALID("field2", "", u"Pole nie mo\u017Ce by\u0107 puste");
	PaletteOverride{.window = Qt::white, .windowText = Qt::black}.install(&page);
	ui.restyleFields();
	QCOMPARE_STR(page.findChild<QWidget*>("field1Edit")->styleSheet(), "");
	QCOMPARE_STR(page.findChild<QWidget*>("field2Edit")->styleSheet(), "background-color: #ffcccc");

	PaletteOverride{.window = Qt::black, .windowText = Qt::white}.install(&page);
	ui.restyleFields();
	QCOMPARE_STR(page.findChild<QWidget*>("field1Edit")->styleSheet(), "");
	QCOMPARE_STR(page.findChild<QWidget*>("field2Edit")->styleSheet(), "background-color: #551111");

	QVERIFY_EDIT_TEXT_IS_INVALID("field2", "12345678901234567890", u"Pole jest niepoprawne");
	QVERIFY_EDIT_TEXT_IS_VALID("field2", "123456789");

#define SIX_DIGITS u"Tytu\u0142 ubezpieczenia powinien sk\u0142ada\u0107 si\u0119 z 6 cyfr w formacie \"0000 0 0\""
	QVERIFY_EDIT_TEXT_IS_INVALID("field3", "999 9 9", SIX_DIGITS);
	QVERIFY_EDIT_TEXT_IS_INVALID("field3", "999 9 999", SIX_DIGITS);
	QVERIFY_EDIT_TEXT_IS_INVALID("field3", "abcd 9 9", SIX_DIGITS);
	QVERIFY_EDIT_TEXT_IS_VALID("field3", "0110 2 3");

	ui.readValue(data);

	QCOMPARE_EQ(data.field_1, "something else"sv);
	QCOMPARE_EQ(data.field_2, "123456789"sv);
	QCOMPARE_EQ(data.field_3.title_code, "0110"sv);
	QCOMPARE_EQ(data.field_3.pension_right, 2);
	QCOMPARE_EQ(data.field_3.disability_level, 3);
}

void ControlsTest::Forms_documentComboBox() {
	TestPage page{};
	DocumentUI ui{};
	ui.setupPageUI(&page);
	person data{};
	ui.attach(data);

	std::string lastBlockCheckedKind{};
	std::string lastBlockCheckedDocument{};
	std::optional<std::string> blockingLabel{};

	ui.get<DocumentABCCombo>().setBlockChecker([&lastBlockCheckedKind, &lastBlockCheckedDocument, &blockingLabel](
	                                               std::string_view kind, std::string_view number) {
		lastBlockCheckedKind = kind;
		lastBlockCheckedDocument = number;
		return blockingLabel;
	});

	ui.get<DocumentABCCombo>().restyleField(false);

	QVERIFY_COMBO_BOX("documentKind", "&Rodzaj dokumentu", -1, "A - Klucz A", "B - Klucz B", "C - Klucz C");
	QVERIFY_LINE_EDIT("document", "&Seria i numer dokumentu", "");

	data.kind = "B"sv;
	data.document = "AA?000000"sv;
	ui.attach(data);

	QVERIFY_COMBO_BOX("documentKind", "&Rodzaj dokumentu", 1, "A - Klucz A", "B - Klucz B", "C - Klucz C");
	QVERIFY_LINE_EDIT("document", "&Seria i numer dokumentu", "AA?000000");

	SELECT_OPTION("documentKind", 0);
	QVERIFY_IS_INVALID("document", u"Pole nie mo\u017Ce by\u0107 puste");
	QVERIFY_EDIT_TEXT_IS_INVALID("document", "AA", u"Klucz A musi mie\u0107 poprawn\u0105 sum\u0119 kontroln\u0105");
	QVERIFY(lastBlockCheckedKind.empty());
	QVERIFY(lastBlockCheckedDocument.empty());

	QVERIFY_EDIT_TEXT_IS_VALID("document", "AAA000000");
	QCOMPARE_EQ(lastBlockCheckedKind, "A"sv);
	QCOMPARE_EQ(lastBlockCheckedDocument, "AAA000000"sv);

	blockingLabel = "Another Name"s;
	QVERIFY_EDIT_TEXT_IS_INVALID("document", "ZZZ499999",
	                             u"Another Name: znaleziono inn\u0105 ubezpieczon\u0105 osob\u0119 z tym dokumentem");
	QCOMPARE_EQ(lastBlockCheckedKind, "A"sv);
	QCOMPARE_EQ(lastBlockCheckedDocument, "ZZZ499999"sv);
	blockingLabel.reset();

	SELECT_OPTION("documentKind", 2);
	QVERIFY_IS_INVALID_WITH_TEXT("document", "", u"Pole nie mo\u017Ce by\u0107 puste");
	QVERIFY_EDIT_TEXT_IS_VALID("document", "26211012346");
	QCOMPARE_EQ(lastBlockCheckedKind, "C"sv);
	QCOMPARE_EQ(lastBlockCheckedDocument, "26211012346"sv);

	SELECT_OPTION("documentKind", 0);
	QVERIFY_IS_VALID_WITH_TEXT("document", "ZZZ499999");
	QCOMPARE_EQ(lastBlockCheckedKind, "A"sv);
	QCOMPARE_EQ(lastBlockCheckedDocument, "ZZZ499999"sv);

	ui.readValue(data);
	QCOMPARE_EQ(data.kind, "A"sv);
	QCOMPARE_EQ(data.document, "ZZZ499999"sv);
}

void ControlsTest::Forms_tableView() {
	TestPage page{};
	HistoryUI ui{};
	ui.setupPageUI(&page);
	insured_type data{};

	QVERIFY(page.findChild<QWidget*>("employmentHistoryAddButton"));
	QVERIFY(page.findChild<QWidget*>("employmentHistoryRemoveButton"));
	QVERIFY(page.findChild<QWidget*>("employmentHistoryTreeView"));

	auto view = page.findChild<QTreeView*>("employmentHistoryTreeView");
	{
		auto oldModel = view->model();
		view->setModel(new TestableModel{});
		oldModel->deleteLater();
	}
	auto model = view->model();

	ui.attach(data);
	static_cast<HistoryListView::Model*>(model)->replaceRows(std::vector<insured_type::history_type>{
	    {.since = 2011y / January, .part_time_scale = {5, 6}, .salary = 8000_PLN},
	    {.since = 1899y / January, .part_time_scale = {3, 4}, .salary = minimal_salary},
	    {.since = 2012y / January, .part_time_scale = {3, 4}, .salary = 10648_PLN},
	    {.since = 2013y / January, .part_time_scale = {4, 5}, .salary = 9680_PLN},
	    {.since = 2014y / January, .part_time_scale = {5, 6}, .salary = 8800_PLN},
	    {.since = 2015y / January, .part_time_scale = {1, 1}, .salary = 11712.80_PLN},
	});

	QCOMPARE_EQ(model->headerData(0, Qt::Horizontal), QString{u"Początek"});
	QCOMPARE_EQ(model->headerData(1, Qt::Horizontal), QString{u"Część"});
	QCOMPARE_EQ(model->headerData(2, Qt::Horizontal), QString{u"Pensja"});
	QCOMPARE_EQ(model->headerData(3, Qt::Horizontal), QVariant{});
	QCOMPARE_EQ(model->headerData(0, Qt::Vertical), QVariant{});
	QCOMPARE_EQ(model->headerData(0, Qt::Horizontal, Qt::EditRole), QVariant{});

	QCOMPARE_EQ(data.history, (std::vector<insured_type::history_type>{
	                              {.since = 1899y / January, .part_time_scale = {3, 4}, .salary = minimal_salary},
	                              {.since = 2011y / January, .part_time_scale = {5, 6}, .salary = 8000_PLN},
	                              {.since = 2012y / January, .part_time_scale = {3, 4}, .salary = 10648_PLN},
	                              {.since = 2013y / January, .part_time_scale = {4, 5}, .salary = 9680_PLN},
	                              {.since = 2014y / January, .part_time_scale = {5, 6}, .salary = 8800_PLN},
	                              {.since = 2015y / January, .part_time_scale = {1, 1}, .salary = 11712.80_PLN},
	                          }));
	view->sortByColumn(0, Qt::DescendingOrder);
	QCOMPARE_EQ(data.history, (std::vector<insured_type::history_type>{
	                              {.since = 2015y / January, .part_time_scale = {1, 1}, .salary = 11712.80_PLN},
	                              {.since = 2014y / January, .part_time_scale = {5, 6}, .salary = 8800_PLN},
	                              {.since = 2013y / January, .part_time_scale = {4, 5}, .salary = 9680_PLN},
	                              {.since = 2012y / January, .part_time_scale = {3, 4}, .salary = 10648_PLN},
	                              {.since = 2011y / January, .part_time_scale = {5, 6}, .salary = 8000_PLN},
	                              {.since = 1899y / January, .part_time_scale = {3, 4}, .salary = minimal_salary},
	                          }));
	view->sortByColumn(1, Qt::AscendingOrder);
	QCOMPARE_EQ(data.history, (std::vector<insured_type::history_type>{
	                              {.since = 2012y / January, .part_time_scale = {3, 4}, .salary = 10648_PLN},
	                              {.since = 1899y / January, .part_time_scale = {3, 4}, .salary = minimal_salary},
	                              {.since = 2013y / January, .part_time_scale = {4, 5}, .salary = 9680_PLN},
	                              {.since = 2014y / January, .part_time_scale = {5, 6}, .salary = 8800_PLN},
	                              {.since = 2011y / January, .part_time_scale = {5, 6}, .salary = 8000_PLN},
	                              {.since = 2015y / January, .part_time_scale = {1, 1}, .salary = 11712.80_PLN},
	                          }));
	view->sortByColumn(0, Qt::AscendingOrder);
	QCOMPARE_EQ(data.history, (std::vector<insured_type::history_type>{
	                              {.since = 1899y / January, .part_time_scale = {3, 4}, .salary = minimal_salary},
	                              {.since = 2011y / January, .part_time_scale = {5, 6}, .salary = 8000_PLN},
	                              {.since = 2012y / January, .part_time_scale = {3, 4}, .salary = 10648_PLN},
	                              {.since = 2013y / January, .part_time_scale = {4, 5}, .salary = 9680_PLN},
	                              {.since = 2014y / January, .part_time_scale = {5, 6}, .salary = 8800_PLN},
	                              {.since = 2015y / January, .part_time_scale = {1, 1}, .salary = 11712.80_PLN},
	                          }));
	model->sort(3);
	QCOMPARE_EQ(data.history, (std::vector<insured_type::history_type>{
	                              {.since = 1899y / January, .part_time_scale = {3, 4}, .salary = minimal_salary},
	                              {.since = 2011y / January, .part_time_scale = {5, 6}, .salary = 8000_PLN},
	                              {.since = 2012y / January, .part_time_scale = {3, 4}, .salary = 10648_PLN},
	                              {.since = 2013y / January, .part_time_scale = {4, 5}, .salary = 9680_PLN},
	                              {.since = 2014y / January, .part_time_scale = {5, 6}, .salary = 8800_PLN},
	                              {.since = 2015y / January, .part_time_scale = {1, 1}, .salary = 11712.80_PLN},
	                          }));

	DataSet const display = {
	    {QString{"Od zawsze"}, QString{u"¾"}, QString{u"Minimalna"}},
	    {QString{"sty 2011"}, QString{u"⅚"}, QString{u"8" GROUP_SEP "000,00" NBSP "zł"}},
	    {QString{"sty 2012"}, QString{u"¾"}, QString{u"10" GROUP_SEP "648,00" NBSP "zł"}},
	    {QString{"sty 2013"}, QString{u"⅘"}, QString{u"9" GROUP_SEP "680,00" NBSP "zł"}},
	    {QString{"sty 2014"}, QString{u"⅚"}, QString{u"8" GROUP_SEP "800,00" NBSP "zł"}},
	    {QString{"sty 2015"}, QString{u"Pełny"}, QString{u"11" GROUP_SEP "712,80" NBSP "zł"}},
	};
	QCOMPARE_EQ(itemData(model, Qt::DisplayRole), display);

	DataSet const edit = {
	    {QString{"1899/01"}, QString{"3/4"}, QString{u"1,00"}},
	    {QString{"2011/01"}, QString{"5/6"}, QString{u"8" GROUP_SEP "000,00"}},
	    {QString{"2012/01"}, QString{"3/4"}, QString{u"10" GROUP_SEP "648,00"}},
	    {QString{"2013/01"}, QString{"4/5"}, QString{u"9" GROUP_SEP "680,00"}},
	    {QString{"2014/01"}, QString{"5/6"}, QString{u"8" GROUP_SEP "800,00"}},
	    {QString{"2015/01"}, QString{"1/1"}, QString{u"11" GROUP_SEP "712,80"}},
	};
	QCOMPARE_EQ(itemData(model, Qt::EditRole), edit);

	DataSet const alignment = {
	    {Qt::AlignRight, Qt::AlignRight, Qt::AlignRight}, {Qt::AlignRight, Qt::AlignRight, Qt::AlignRight},
	    {Qt::AlignRight, Qt::AlignRight, Qt::AlignRight}, {Qt::AlignRight, Qt::AlignRight, Qt::AlignRight},
	    {Qt::AlignRight, Qt::AlignRight, Qt::AlignRight}, {Qt::AlignRight, Qt::AlignRight, Qt::AlignRight},
	};
	QCOMPARE_EQ(itemData(model, Qt::TextAlignmentRole), alignment);

	DataSet const none = {
	    {{}, {}, {}}, {{}, {}, {}}, {{}, {}, {}}, {{}, {}, {}}, {{}, {}, {}}, {{}, {}, {}},
	};
	QCOMPARE_EQ(itemData(model, Qt::FontRole), none);

	QVERIFY2(!model->index(-1, 2).isValid(), "there is not index for bad refs");
	QVERIFY2(!model->index(0, model->rowCount() + 2).isValid(), "there is not index for bad refs");
	QVERIFY2(!model->index(0, 0, model->index(0, 0)).isValid(), "this is not a tree");
	QVERIFY2(!model->parent(model->index(0, 0)).isValid(), "parent of every index is the root index");
	QCOMPARE_EQ(model->flags({}), Qt::ItemFlags{});
	QCOMPARE_EQ(model->flags(model->index(0, 0)), Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);

	int newRow = static_cast<HistoryListView::Model*>(model)->addNewRow();

	auto const today = get_today();
	auto const nextSince = today.year() / today.month() + months{1};

	QCOMPARE_EQ(data.history.size(), 7);
	QCOMPARE_EQ(data.history.back().since, nextSince);
	QCOMPARE_EQ(data.history.back().part_time_scale.num, 1);
	QCOMPARE_EQ(data.history.back().part_time_scale.den, 1);
	QCOMPARE_EQ(data.history.back().salary, minimal_salary);

#define QCOMPARE_FIELD_(editText, value, col, access)                               \
	do {                                                                            \
		model->setData(model->index(newRow, col), QString{editText}, Qt::EditRole); \
		QCOMPARE_EQ(data.history.back().access, value);                             \
	} while (false)

#define QCOMPARE_SINCE(editText, value) QCOMPARE_FIELD_(editText, value, 0, since)
#define QCOMPARE_RATIO(editText, numerator, denominator)                   \
	do {                                                                   \
		QCOMPARE_FIELD_(editText, numerator, 1, part_time_scale.num);      \
		QCOMPARE_EQ(data.history.back().part_time_scale.den, denominator); \
	} while (false)

#define QCOMPARE_SALARY(editText, value) QCOMPARE_FIELD_(editText, value, 2, salary)

	QCOMPARE_SINCE("bla/bla", nextSince);
	QCOMPARE_SINCE("2025/12", 2025y / 12);
	QCOMPARE_SINCE("2012/01", 2025y / 12);  // must be unique

	QCOMPARE_RATIO("12/16", 3, 4);
	QCOMPARE_RATIO("1/0", 3, 4);
	QCOMPARE_RATIO("1/1", 1, 1);
	QCOMPARE_RATIO("something", 1, 1);

	QCOMPARE_SALARY("minimalny", minimal_salary);
	QCOMPARE_SALARY("1234", 1234_PLN);
	QCOMPARE_SALARY("minimalna", minimal_salary);
	QCOMPARE_SALARY("2300 zł", 2300_PLN);
	QCOMPARE_SALARY("minimalne", minimal_salary);
	QCOMPARE_SALARY("43 543,27 pln", 43543.27_PLN);
	QCOMPARE_SALARY("minimal", minimal_salary);
	QCOMPARE_SALARY("123 pln", 123_PLN);
	QCOMPARE_SALARY("-1", minimal_salary);
	QCOMPARE_SALARY("500 PLN", 500_PLN);
	QCOMPARE_SALARY("error", 500_PLN);

	auto const invIndex = static_cast<TestableModel*>(model)->makeInvalidIndex(newRow, 3);
	QVERIFY2(!model->setData(model->index(newRow, 0), QString{"value"}, Qt::DisplayRole),
	         "only edit role is supported");
	QVERIFY2(!model->setData(invIndex, QString{"value"}, Qt::EditRole), "out of bounds");
	QCOMPARE_EQ(model->data(invIndex, Qt::EditRole), QVariant{});

	QVERIFY2(ui.isValid(), "should always be valid");

	insured_type copy{};
	QVERIFY2((ui.readValue(copy), copy.history.empty()), "all data is modified in-place");
}

#include "Forms.test.moc"
