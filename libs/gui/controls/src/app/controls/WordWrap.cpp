// Copyright (c) 2026 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <QPainter>
#include <QRegularExpression>
#include <algorithm>
#include <app/controls/WordWrap.hpp>
#include <print>
#include <utility>

// define DEBUG_MD

namespace quick_dra::gui {
	namespace {
		bool isSpace(QChar ch) {
			switch (ch.unicode()) {
				case u'\u00a0':
				case u'\u202F':
					return false;
				default:
					break;
			}
			return ch.isSpace();
		}

		struct Splitter {
			QList<LineInfo> result{};
			qsizetype wordIndex{};
			qreal yMin{};
			qreal yMax{};

			void onWord(bool firstWord,
			            LabelInfo::Word const& word,
			            TextPosition const& pos,
			            LabelInfo const& label,
			            [[maybe_unused]] QStringView const& text) {
				if (firstWord) {
					result.emplace_back();
				}
				auto& line = result.back();
				line.pos.includeSpace(label.space);
				line.pos.join(pos);

#ifdef DEBUG_MD
				std::print("l:[");
				for (auto const& span : word.spans) {
					std::print("{}", span.letters.sliceInto(text).toUtf8().toStdString());
				}
				std::print("] advance: {} box: {}, {} [{}]\n", line.pos.advance, line.pos.boundingBox.left(),
				           line.pos.boundingBox.right(), line.pos.boundingBox.width());
#endif

				for (auto const& span : word.spans) {
					line.lineHeight.extend(label.info(span).lineHeight);
				}

				line.wordIndexes.emplace_back(wordIndex);
				++wordIndex;
			}

			static TextPosition wordReach(LabelInfo::Word const& word, [[maybe_unused]] QStringView const& text) {
				TextPosition result{};

				for (auto const& span : word.spans) {
					result.join(span.pos);
				}

#ifdef DEBUG_MD
				std::print("w:[");
				for (auto const& span : word.spans) {
					std::print("{}", span.letters.sliceInto(text).toUtf8().toStdString());
				}
				std::print("] advance: {} box: {}, {} [{}]\n", result.advance, result.boundingBox.left(),
				           result.boundingBox.right(), result.boundingBox.width());
#endif
				return result;
			}

			Splitter&& split(int width, LabelInfo const& label, QStringView const& text) {
				qreal advance{}, xMin{}, xMax{};
				auto firstWord{true};
				for (auto const& word : label.words) {
					auto const pos = wordReach(word, text);
					xMin = std::min(xMin, advance + pos.boundingBox.left());
					xMax = std::max(xMax, advance + pos.boundingBox.right());

					if ((xMax - xMin) > width && !firstWord) {
						firstWord = true;
						advance = 0;
						xMin = std::min(advance, pos.boundingBox.left());
						xMax = pos.boundingBox.right();
					}

					onWord(firstWord, word, pos, label, text);

					advance += pos.advance + label.space;
					firstWord = false;
				}

				return std::move(*this);
			}
		};

		struct Mapping {
			QStringView needle{};
			bool isBold{};

			std::pair<QString, QList<StringSpan>> parse(QStringView const& view) const;
		};

		constexpr QStringView operator""_sv(char16_t const* ptr, size_t size) {
			return QStringView{ptr, static_cast<qsizetype>(size)};
		}

		static constexpr auto markdown_bold_italic = std::array{
		    Mapping{.needle = u"**"_sv, .isBold = true},
		    Mapping{.needle = u"__"_sv, .isBold = true},
		    Mapping{.needle = u"*"_sv, .isBold = false},
		    Mapping{.needle = u"_"_sv, .isBold = false},
		};

		std::pair<QString, QList<StringSpan>> Mapping::parse(QStringView const& view) const {
			auto items = view.tokenize(needle).toContainer<QList<QStringView>>();

			std::pair<QString, QList<StringSpan>> result{};
			auto& [output, events] = result;

			auto activation = 1;

			qsizetype size = 0;
			auto valid = items.size();
			for (qsizetype index = 0; index < valid; ++index) {
				auto const& item = items[index];
				auto active = index % 2 == activation;
				size += item.size();
				if (item.empty() && active) {
					size += needle.size();
					activation = (activation + 1) % 2;
				}
			}
			auto const unfinished = items.size() % 2 != activation;
			if (unfinished) size += needle.size();
			output.reserve(size);

			activation = 1;
			if (unfinished) --valid;
			for (qsizetype index = 0; index < valid; ++index) {
				auto const& item = items[index];
				auto active = index % 2 == activation;
				auto empty = active && item.empty();
				if (empty) {
					activation = (activation + 1) % 2;
					output.append(needle);
					continue;
				}

				if (active) {
					events.push_back({
					    .offset = output.size(),
					    .length = item.size(),
					});
				}
				output.append(item);
			}

			if (unfinished) {
				output.append(needle);
				if (!items.empty()) output.append(items.back());
			}

#ifdef DEBUG_MD
			std::print("[{}]({}) => [{}]", view.toUtf8().toStdString(), needle.toUtf8().toStdString(),
			           result.first.toStdString());
			for (auto const& span : events) {
				std::print(" ({}, {})", span.offset, span.length);
			}
			std::print("\n");
#endif

			return result;
		}

		void adjustSpans(QList<StringSpan>& dst, StringSpan const& span, qsizetype needleLength) {
			auto const min = span.min();
			auto const max = span.max();
			for (auto& adjusted : dst) {
#ifdef DEBUG_MD
				std::print("({}, {})/({}, {}) ", span.offset, span.length, adjusted.offset, adjusted.length);
#endif
				if (adjusted.min() < min) {
#ifdef DEBUG_MD
					std::print("(adjusted.min < span.min) ");
#endif
					// 0       0
					// S------->
					//            S----------->

					// 0           -N
					// S------------>
					//        S----------->
					if (adjusted.max() >= min) {
#ifdef DEBUG_MD
						std::print("(adjusted.max >= span.min) [L-{}] ", needleLength);
#endif
						adjusted.length -= needleLength;
					}

					// 0                    -2N
					// S--------------------->
					//      S------->
					if (adjusted.max() >= max) {
#ifdef DEBUG_MD
						std::print("(adjusted.max >= span.max) [L-{}] ", needleLength);
#endif
						adjusted.length -= needleLength;
					}
				} else if (adjusted.min() == min) {
					// 0      -N
					// S------->
					// S----------->

					// 0          -N
					// S----------->
					// S----------->

					// 0              -2N
					// S---------------->
					// S----------->
#ifdef DEBUG_MD
					std::print("(adjusted.min == span.min) [L-{}] ", needleLength);
#endif
					adjusted.length -= needleLength;
					if (adjusted.max() > max) {
						adjusted.length -= needleLength;
					}
				} else {
					//    -N     0
					//     S----->
					//  S----------->
#ifdef DEBUG_MD
					std::print("(adjusted.min > span.min) [O-{}] ", needleLength);
#endif
					adjusted.offset -= needleLength;
					if (adjusted.min() >= max) {
						//        -2N      0
						//         S------->
						// S---->
#ifdef DEBUG_MD
						std::print("(adjusted.min >= span.max) [O-{}] ", needleLength);
#endif
						adjusted.offset -= needleLength;
					} else if (adjusted.max() >= max) {
						//   -N      -N
						//    S------->
						// S---->
#ifdef DEBUG_MD
						std::print("(adjusted.max >= span.max) [L-{}] ", needleLength);
#endif
						adjusted.length -= needleLength;
					}
				}

#ifdef DEBUG_MD
				std::print("=> ({}, {})\n", adjusted.offset, adjusted.length);
#endif
			}
		}

		void merge(QList<StringSpan>& dst, QList<StringSpan> const& src, QStringView const&) {
			dst.append(src);
			std::sort(dst.begin(), dst.end());

			auto ref = dst.begin();
			auto it = ref;
			auto end = dst.end();
			if (it != end) ++it;
			while (it != end) {
				if (ref->intersectsWith(*it)) {
					*ref = ref->unionWith(*it);
					it->length = 0;
				} else {
					ref = it;
				}
				++it;
			}

			end = std::remove_if(dst.begin(), dst.end(), [](auto const& span) { return span.length == 0; });
			dst.erase(end, dst.end());
		}

		QList<LabelInfo::Span> applyMarkup(QList<LabelInfo::Span> currentLayer,
		                                   QList<StringSpan> const& style,
		                                   [[maybe_unused]] QStringView const& text,
		                                   auto&& modify) {
			for (auto const& ref : style) {
#ifdef DEBUG_MD
				bool printed = false;
#endif
				QList<LabelInfo::Span> nextLayer{};
				nextLayer.reserve(currentLayer.size() * 3);  // maximal possible split
				for (auto const& cell : currentLayer) {
					if (!ref.intersectsWith(cell.letters)) {
						nextLayer.push_back(cell);
						continue;
					}
					auto const intersection = ref.intersectionWith(cell.letters);
#ifdef DEBUG_MD
					if (!printed) {
						printed = true;
						std::print("[ref: {}, {} ({})]", ref.offset, ref.length,
						           ref.sliceInto(text).toUtf8().toStdString());
					}
					std::print(" target:{}, {} ({})", cell.letters.offset, cell.letters.length,
					           cell.letters.sliceInto(text).toUtf8().toStdString());
#endif

					if (intersection.min() > cell.letters.min()) {
						auto copy = cell;
						copy.letters = cell.letters.before(intersection);
#ifdef DEBUG_MD
						std::print(" before:{}, {} ({})", copy.letters.offset, copy.letters.length,
						           copy.letters.sliceInto(text).toUtf8().toStdString());
#endif
						nextLayer.push_back(copy);
					}

					{
						auto copy = cell;
						copy.letters = intersection;
						modify(copy);
#ifdef DEBUG_MD
						std::print(" mod:{}, {} ({})", copy.letters.offset, copy.letters.length,
						           copy.letters.sliceInto(text).toUtf8().toStdString());
#endif
						nextLayer.push_back(copy);
					}

					if (intersection.max() < cell.letters.max()) {
						auto copy = cell;
						copy.letters = cell.letters.after(intersection);
#ifdef DEBUG_MD
						std::print(" after:{}, {} ({})", copy.letters.offset, copy.letters.length,
						           copy.letters.sliceInto(text).toUtf8().toStdString());
#endif
						nextLayer.push_back(copy);
					}
				}
				std::swap(currentLayer, nextLayer);
#ifdef DEBUG_MD
				if (printed) std::print("\n");
#endif
			}

			return currentLayer;
		}

		LabelInfo::Word applyMarkup(StringSpan const& letters, Formatted const& markdown, QStringView const& text) {
			auto strong = applyMarkup({{.letters = letters}}, markdown.bold, text,
			                          [](LabelInfo::Span& span) { span.isBold = true; });
			return {.spans = applyMarkup(std::move(strong), markdown.italic, text,
			                             [](LabelInfo::Span& span) { span.isItalic = true; })};
		}  // GCOV_EXCL_LINE[WIN32]
	}  // namespace

	QList<StringSpan> splitForWordWrap(QStringView const& text) {
		QList<StringSpan> result{};
		qsizetype offset{};
		auto it = text.begin();
		auto const end = text.end();

		while (it != end && isSpace(*it))
			++it, ++offset;

		while (it != end) {
			qsizetype length{};
			while (it != end && !isSpace(*it))
				++it, ++length;

			result.emplace_back(StringSpan{.offset = offset, .length = length});
			offset += length;
			while (it != end && isSpace(*it))
				++it, ++offset;
		}

		return result;
	}  // GCOV_EXCL_LINE[GCC]

	Formatted markdownFormat(QStringView const& text) {
		Formatted result{};
		auto view = text;

		for (auto const& map : markdown_bold_italic) {
			QList<StringSpan> spans{};
			std::tie(result.text, spans) = map.parse(view);
			view = result.text;

			auto const needleLength = map.needle.size();
			for (auto const& src : spans) {
				adjustSpans(result.bold, src, needleLength);
				adjustSpans(result.italic, src, needleLength);
			}

			merge(map.isBold ? result.bold : result.italic, spans, result.text);
		}

		return result;
	}  // GCOV_EXCL_LINE[GCC]

	void LineHeight::measure(QFontMetricsF const& fm) {
		ascent = fm.ascent();
		descent = fm.descent();
		leading = fm.leading();
	}

	void LineHeight::extend(LineHeight const& rhs) {
		ascent = std::max(ascent, rhs.ascent);
		descent = std::max(descent, rhs.descent);
		leading = std::max(leading, rhs.leading);
	}

	void TextPosition::includeSpace(qreal width) noexcept {
		if (boundingBox.isEmpty()) return;
		join({.advance = width});
	}

	void TextPosition::join(TextPosition const& rhs) noexcept {
		auto const xFrom = rhs.boundingBox.left();
		auto const xTo = std::max(rhs.boundingBox.right(), rhs.advance);
		auto const xMin = std::min(boundingBox.left(), advance + xFrom);
		auto const xMax = std::max(boundingBox.right(), advance + xTo);
		advance += rhs.advance;

		boundingBox.setLeft(xMin);
		boundingBox.setRight(xMax);
		boundingBox.setTop(std::min(boundingBox.top(), rhs.boundingBox.top()));
		boundingBox.setBottom(std::max(boundingBox.bottom(), rhs.boundingBox.bottom()));
	}

	void LabelInfo::Span::measure(QFontMetricsF const& fm, QStringView const& text, bool bold, bool italic) {
		if (bold != isBold || italic != isItalic) {
			return;
		}
		auto const str = QString{letters.sliceInto(text)};
		pos.advance = fm.horizontalAdvance(str);
		pos.boundingBox = fm.boundingRect(str);
#ifdef DEBUG_MD
		std::print("m:[{}] advance: {} box: {}, {} [{}]\n", str.toStdString(), pos.advance, pos.boundingBox.left(),
		           pos.boundingBox.right(), pos.boundingBox.width());
#endif
	}

	void LabelInfo::Word::measure(QFontMetricsF const& fm, QStringView const& text, bool bold, bool italic) {
		for (auto& span : spans) {
			span.measure(fm, text, bold, italic);
		}
	}

	void LabelInfo::parse(QStringView newText, bool isHeading) {
		auto markup = markdownFormat(newText);

		if (isHeading) {
			markup.bold.clear();
			markup.bold.push_back({.length = markup.text.size()});
		}

		markdownClearedText = std::move(markup.text);
		auto const apply = [&markup, this](auto const& letters) {
			return applyMarkup(letters, markup, markdownClearedText);
		};

		auto const spans = splitForWordWrap(markdownClearedText);
		QList<Word> result{};
		result.reserve(spans.size());
		std::transform(spans.begin(), spans.end(), std::back_inserter(result), apply);
		std::swap(words, result);
		dirty = true;
	}

	void LabelInfo::measure(QFont const& font) {
		if (!dirty) return;

		space = QFontMetricsF{font}.horizontalAdvance(u' ');
		dirty = false;

		for (auto& fontInfo : fontInfos) {
			fontInfo.used = false;
		}

		for (auto const& word : words) {
			for (auto const& span : word.spans) {
				info(span).used = true;
			}
		}

		for (auto& fontInfo : fontInfos) {
			if (!fontInfo.used) {
				continue;
			}

			auto copy = font;
			copy.setBold(fontInfo.isBold);
			copy.setItalic(fontInfo.isItalic);

			QFontMetricsF fm{copy};
			fontInfo.lineHeight.measure(fm);
			for (auto& word : words) {
				word.measure(fm, markdownClearedText, fontInfo.isBold, fontInfo.isItalic);
			}
		}
	}

	QList<LineInfo> LabelInfo::splitIntoLines(int width) {
		return Splitter{}.split(width, *this, markdownClearedText).result;
	}

	QSizeF LabelInfo::boundingBox(QList<LineInfo> const& lines) const noexcept {
		qreal baseline{}, yMin{}, yMax{}, widthF{};
		for (auto const& [height, pos, refs] : lines) {
			yMin = std::min(yMin, baseline + pos.boundingBox.top());
			yMax = std::max(yMax, baseline + pos.boundingBox.bottom());
			widthF = std::max(widthF, pos.boundingBox.width());
			baseline += height.lineSpacing();
		}

		return {widthF, yMax - yMin};
	}

	QSizeF LabelInfo::singleWordSize() const noexcept {
		qreal baseline{}, yMin{}, yMax{}, widthF{};
		for (auto const& word : words) {
			auto const pos = Splitter::wordReach(word, markdownClearedText);
			yMin = std::min(yMin, baseline + pos.boundingBox.top());
			yMax = std::max(yMax, baseline + pos.boundingBox.bottom());
			widthF = std::max(widthF, pos.boundingBox.width());

			LineHeight height{};
			for (auto const& span : word.spans) {
				height.extend(info(span).lineHeight);
			}
			baseline += height.lineSpacing();
		}
		return {widthF, yMax - yMin};
	}

	QSizeF LabelInfo::singleLineSize() const noexcept {
		qreal ascent{}, descent{};
		for (auto const& font : fontInfos) {
			ascent = std::max(ascent, font.used * font.lineHeight.ascent);
			descent = std::max(descent, font.used * font.lineHeight.descent);
		}

		TextPosition line{.boundingBox{0, -ascent, 0, ascent + descent}};
		for (auto const& word : words) {
			auto const pos = Splitter::wordReach(word, markdownClearedText);

			line.includeSpace(space);
			line.join(pos);
		}

		return line.boundingBox.size();
	}

	void LabelInfo::paint(QPainter* painter, QList<LineInfo> const& lines) const noexcept {
		auto font = painter->font();
		for (auto const& info : fontInfos) {
			if (!info.used) continue;
			font.setBold(info.isBold);
			font.setItalic(info.isItalic);
			painter->setFont(font);
			paint(painter, lines, info.isBold, info.isItalic);
		}
	}

	void LabelInfo::paint(QPainter* painter, QList<LineInfo> const& lines, bool isBold, bool isItalic) const noexcept {
		qreal offset = 0;
		if (!lines.empty()) {
			auto const& front = lines.front();
			offset = std::max(front.lineHeight.ascent, -front.pos.boundingBox.top());
			offset -= front.lineHeight.ascent;
		}

		auto baseline = offset;
		for (auto const& [height, box, refs] : lines) {
			baseline += height.ascent;

			offset = 0;
			if (!refs.empty()) {
				auto const& word = words[refs.front()];
				auto const& rect = word.spans.front().pos.boundingBox;
				offset = std::max(offset, -rect.left());
			}

			auto advance = offset;
			for (auto const wordRef : refs) {
				auto const& word = words.at(wordRef);

				for (auto const& span : word.spans) {
#ifdef DEBUG_MD
					std::print("p:[{}] advance: {} baseline: {} advance: {} box: {}, {} [{}]\n",
					           span.letters.sliceInto(markdownClearedText).toUtf8().toStdString(), advance, baseline,
					           span.pos.advance, span.pos.boundingBox.left(), span.pos.boundingBox.right(),
					           span.pos.boundingBox.width());
#endif
					if (span.isBold == isBold && span.isItalic == isItalic) {
						auto const view = span.letters.sliceInto(markdownClearedText);

						painter->drawText(QPointF{advance, baseline}, QString{view});
					}

					advance += span.pos.advance;
				}
#ifdef DEBUG_MD
				std::print("p:[SP] advance: {} baseline: {} advance: {}\n", advance, baseline, space);
#endif

				advance += space;
			}

			baseline += height.descent + height.leading;
		}
	}

	void MarkdownLabel::setFont(QFont const& font) noexcept {
		font_ = font;
		needsWork(Measure);
	}

	void MarkdownLabel::setWidth(int width) noexcept {
		if (currentWidth_ == width) return;
		currentWidth_ = width;
		needsWork(Layout);
	}

	void MarkdownLabel::moveTo(QPointF const& topLeft) noexcept { topLeft_ = topLeft; }

	void MarkdownLabel::setFontSize(qreal scale) noexcept {
		if (fontScale_ == scale) return;
		fontScale_ = scale;
		needsWork(Measure);
	}

	void MarkdownLabel::setIsHeading(bool value) noexcept {
		if (isHeading_ == value) return;
		isHeading_ = value;
		needsWork(Parse);
	}

	void MarkdownLabel::setText(QString const& newText) {
		if (text_ == newText) return;
		text_ = newText;
		needsWork(Parse);
	}

	void MarkdownLabel::parse() { processChanges(Measure); }

	QSizeF const& MarkdownLabel::calcSize() {
		processChanges();
		return size_;
	}

	QSizeF const& MarkdownLabel::calcSingleLineSize() {
		processChanges();
		return singleLineSize_;
	}

	QSizeF MarkdownLabel::calcMinimalSize() {
		processChanges();
		return {longestWord_, singleLineSize_.height()};
	}

	QSizeF MarkdownLabel::calcNarrowest() {
		processChanges(Layout);
		return info_.singleWordSize();
	}

	int MarkdownLabel::heightForWidth(int width) {
		processChanges(Layout);
		auto const lines = info_.splitIntoLines(width);
		auto const size = info_.boundingBox(lines);
		return size.toSize().height();
	}

	void MarkdownLabel::paint(QPainter* painter) {
		processChanges();
		painter->save();
		painter->translate(topLeft_);
		painter->setClipRect(QRectF{QPointF{}, size_}, Qt::IntersectClip);
		painter->setFont(paintFont_);
		info_.paint(painter, lines_);
		painter->restore();
	}

	void MarkdownLabel::processChanges(DirtyLevel until) {
		if (dirty_ >= until) {
			return;
		}

		if (dirty_ == Parse) {
			info_.parse(text_, isHeading_);
			dirty_ = Measure;
			if (until == dirty_) return;
		}

		if (dirty_ == Measure) {
			paintFont_ = font_;
			// 1 in = 96 px, the CSS
			// 1 in = 72 px
			auto const pointSize = paintFont_.pointSize() < 1 ? static_cast<qreal>(paintFont_.pixelSize()) * 4 / 3
			                                                  : paintFont_.pointSizeF();
			paintFont_.setBold(isHeading_);
			paintFont_.setPointSizeF(pointSize * fontScale_);
			info_.measure(paintFont_);
			dirty_ = Layout;
			if (until == dirty_) return;
		}

		if (dirty_ == Layout) {
			lines_ = info_.splitIntoLines(currentWidth_);
			size_ = info_.boundingBox(lines_);
			singleLineSize_ = info_.singleLineSize();
			longestWord_ = info_.singleWordSize().width();
		}

		dirty_ = ReadyForPainting;
	}
};  // namespace quick_dra::gui
