#ifndef TREEWIDGET_HPP
#define TREEWIDGET_HPP

#include <QString>
#include <QTreeWidget>

namespace ResfileViewer
{
	class TreeWidget: public QTreeWidget
	{
		Q_OBJECT

	public:
		explicit TreeWidget(QWidget* parent = 0);

		void add_files(const QStringList& paths);

	protected:
		virtual void dragEnterEvent(QDragEnterEvent* event);
		virtual void dragMoveEvent(QDragMoveEvent* event);
		virtual void dropEvent(QDropEvent* event);
		virtual void contextMenuEvent(QContextMenuEvent* event);

	private Q_SLOTS:
		void extract_files();
	};
}

#endif /* TREEWIDGET_HPP */
