#ifndef __PYTHON_HIGHLIGHTER__
#define __PYTHON_HIGHLIGHTER__


#pragma warning(push)
#pragma warning(disable:4251 4127 4512)
#include <QtGui/QSyntaxHighlighter>
#pragma warning(pop)



class PythonHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	PythonHighlighter(QTextDocument* parent = 0);

protected:
	void highlightBlock(const QString& text);

private:
	struct HighlightingRule
	{
		QRegExp pattern;
		QTextCharFormat format;
	};
	QVector<HighlightingRule> highlightingRules;

	QRegExp commentStartExpression;
	QRegExp commentEndExpression;

	QTextCharFormat keywordFormat;
	QTextCharFormat singleLineCommentFormat;
	QTextCharFormat multiLineCommentFormat;
	QTextCharFormat quotationFormat;
	QTextCharFormat functionFormat;
};


#endif //__PYTHON_HIGHLIGHTER__