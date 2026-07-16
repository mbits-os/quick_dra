// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <QFontMetrics>
#include <QList>
#include <QString>
#include <algorithm>
#include <array>

namespace quick_dra::gui {
	struct StringSpan {
		qsizetype offset{};
		qsizetype length{};  // GCOV_EXCL_LINE

		constexpr auto operator<=>(StringSpan const&) const noexcept = default;
		constexpr bool contains(qsizetype pos) const noexcept { return pos >= offset && (pos - offset) < length; }
		constexpr qsizetype min() const noexcept { return offset; }
		constexpr qsizetype max() const noexcept { return offset + length - 1; }
		constexpr bool intersectsWith(StringSpan const& span) const noexcept {
			return contains(span.min()) || contains(span.max()) || span.contains(min()) || span.contains(max());
		}
		constexpr QStringView sliceInto(QStringView const& text) const noexcept { return text.sliced(offset, length); };
		static constexpr StringSpan fromMinMaxInternal(qsizetype min, qsizetype max) noexcept {
			return {.offset = min, .length = max - min + 1};
		}
		StringSpan unionWith(StringSpan const& rhs) const noexcept {
			return fromMinMaxInternal(std::min(min(), rhs.min()), std::max(max(), rhs.max()));
		}
		StringSpan intersectionWith(StringSpan const& rhs) const noexcept {
			return fromMinMaxInternal(std::max(min(), rhs.min()), std::min(max(), rhs.max()));
		}
		StringSpan before(StringSpan const& rhs) const noexcept { return fromMinMaxInternal(min(), rhs.min() - 1); }
		StringSpan after(StringSpan const& rhs) const noexcept { return fromMinMaxInternal(rhs.max() + 1, max()); }
	};

	QList<StringSpan> splitForWordWrap(QStringView const& text);

	struct Formatted {
		QString text{};
		QList<StringSpan> bold{};
		QList<StringSpan> italic{};
	};

	Formatted markdownFormat(QStringView const&);

	struct LineHeight {
		qreal ascent{};
		qreal descent{};
		qreal leading{};
		qreal lineHeight() const noexcept { return ascent + descent; }
		qreal lineSpacing() const noexcept { return lineHeight() + leading; }
		void measure(QFontMetricsF const& fm);
		void extend(LineHeight const&);
	};

	struct TextPosition {
		QRectF boundingBox{};
		qreal advance{};
		void includeSpace(qreal width) noexcept;
		void join(TextPosition const&) noexcept;
	};

	struct LineInfo {
		LineHeight lineHeight{};
		TextPosition pos{};
		QList<qsizetype> wordIndexes{};
	};

	struct LabelInfo {
		struct Font {
			bool isBold = false;
			bool isItalic = false;
			bool used{};
			LineHeight lineHeight{};
		};

		struct Span {
			StringSpan letters{};
			TextPosition pos{};
			bool isBold : 1 = false;
			bool isItalic : 1 = false;

			void measure(QFontMetricsF const& fm, QStringView const& text, bool bold, bool italic);
		};

		struct Word {
			QList<Span> spans{};

			void measure(QFontMetricsF const& fm, QStringView const& text, bool bold, bool italic);
		};

		QString markdownClearedText{};
		std::array<Font, 4> fontInfos{
		    Font{},
		    Font{.isBold = true},
		    Font{.isItalic = true},
		    Font{.isBold = true, .isItalic = true},
		};
		QList<Word> words{};
		qreal space{};
		bool dirty{};

		void parse(QStringView newText, bool isHeading = false);
		Font& info(Span const& s) noexcept { return fontInfos[fontIndex(s)]; }
		Font const& info(Span const& s) const noexcept { return fontInfos[fontIndex(s)]; }
		void measure(QFont const& font);
		QList<LineInfo> splitIntoLines(int width);
		QSizeF boundingBox(QList<LineInfo> const& lines) const noexcept;
		QSizeF singleWordSize() const noexcept;
		QSizeF singleLineSize() const noexcept;
		void paint(QPainter* painter, QList<LineInfo> const& lines) const noexcept;

	private:
		size_t fontIndex(Span const& s) const noexcept { return (s.isBold ? 1u : 0u) + (s.isItalic ? 2u : 0u); }
		void paint(QPainter* painter, QList<LineInfo> const& lines, bool isBold, bool isItalic) const noexcept;
	};

	class MarkdownLabel {
	public:
		void setFont(QFont const&) noexcept;
		void setWidth(int width) noexcept;
		int width() const noexcept { return currentWidth_; }
		void moveTo(QPointF const&) noexcept;
		QPointF const& topLeft() const noexcept { return topLeft_; }
		void setFontSize(qreal scale = 1) noexcept;
		void setIsHeading(bool value = true) noexcept;
		bool isHeading() const noexcept { return isHeading_; }
		void setText(QString const& newText);
		QString const& text() const noexcept { return text_; }
		QString const& cleanText() const noexcept { return info_.markdownClearedText; }

		void parse();
		QSizeF const& calcSize();
		QSizeF const& calcSingleLineSize();
		QSizeF calcMinimalSize();
		QSizeF calcNarrowest();
		int heightForWidth(int width);
		void paint(QPainter*);

		bool needsLayout() const noexcept { return dirty_ <= Layout; }

	private:
		enum DirtyLevel { Parse, Measure, Layout, ReadyForPainting };

		void needsWork(DirtyLevel level) noexcept {
			if (level < dirty_) dirty_ = level;
		}
		void processChanges(DirtyLevel until = ReadyForPainting);

		QString text_{};
		int currentWidth_{};
		QPointF topLeft_{};
		qreal fontScale_{1};
		DirtyLevel dirty_{Parse};
		bool isHeading_ = false;
		QFont font_{};       // set font
		QFont paintFont_{};  // fontScale_ and isHeading_ applied

		LabelInfo info_{};
		QList<LineInfo> lines_{};
		QSizeF size_{};
		QSizeF singleLineSize_{};
		qreal longestWord_{};
	};
};  // namespace quick_dra::gui
