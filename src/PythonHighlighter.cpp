#include "PythonHighlighter.h"


PythonHighlighter::PythonHighlighter(QTextDocument* parent)
	: QSyntaxHighlighter(parent)
{
	HighlightingRule rule;

	keywordFormat.setForeground(Qt::darkBlue);
	keywordFormat.setFontWeight(QFont::Bold);
	QStringList keywordPatterns;
	keywordPatterns << "\\band\\b"      << "\\bdel\\b"     << "\\bfrom\\b"   << "\\bnot\\b"  << "\\bwhile\\b"
	                << "\\bas\\b"       << "\\belif\\b"    << "\\bglobal\\b" << "\\bor\\b"   << "\\bwith\\b"
	                << "\\bassert\\b"   << "\\belse\\b"    << "\\bif\\b"     << "\\bpass\\b" << "\\byield\\b"
	                << "\\bbreak\\b"    << "\\bexcept\\b"  << "\\bimport\\b" << "\\bprint\\b"
	                << "\\bclass\\b"    << "\\bexec\\b"    << "\\bin\\b"     << "\\braise\\b"
	                << "\\bcontinue\\b" << "\\bfinally\\b" << "\\bis\\b"     << "\\breturn\\b"
	                << "\\bdef\\b"      << "\\bfor\\b"     << "\\blambda\\b" << "\\btry\\b"
	                ;
	foreach(const QString & pattern, keywordPatterns)
	{
		rule.pattern = QRegExp(pattern);
		rule.format = keywordFormat;
		highlightingRules.append(rule);
	}

	quotationFormat.setForeground(Qt::darkGreen);
	rule.pattern = QRegExp("\".*\"");
	rule.format = quotationFormat;
	highlightingRules.append(rule);
	rule.pattern = QRegExp("'.*'");
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	functionFormat.setFontItalic(true);
	functionFormat.setForeground(Qt::blue);
	rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
	rule.format = functionFormat;
	highlightingRules.append(rule);

	singleLineCommentFormat.setForeground(Qt::red);
	rule.pattern = QRegExp("#[^\n]*");
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	multiLineCommentFormat.setForeground(Qt::red);

	commentStartExpression = QRegExp("\"\"\"");
	commentEndExpression = QRegExp("\"\"\"");
}



void PythonHighlighter::highlightBlock(const QString& text)
{
	foreach(const HighlightingRule & rule, highlightingRules)
	{
		QRegExp expression(rule.pattern);
		int index = expression.indexIn(text);
		while(index >= 0)
		{
			int length = expression.matchedLength();
			setFormat(index, length, rule.format);
			index = expression.indexIn(text, index + length);
		}
	}

	setCurrentBlockState(0);

	int startIndex = 0;
	if(previousBlockState() != 1)
		startIndex = commentStartExpression.indexIn(text);

	while(startIndex >= 0)
	{
		int endIndex = commentEndExpression.indexIn(text, startIndex + 3);
		int commentLength;
		if(endIndex == -1)
		{
			setCurrentBlockState(1);
			commentLength = text.length() - startIndex;
		}
		else
		{
			commentLength = endIndex - startIndex
			                + commentEndExpression.matchedLength();
		}
		setFormat(startIndex, commentLength, multiLineCommentFormat);
		startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
	}
}
