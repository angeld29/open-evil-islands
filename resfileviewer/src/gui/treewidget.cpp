#include <algorithm>
#include <functional>
#include <iterator>

#include <boost/shared_ptr.hpp>

#include <QtGlobal>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QList>
#include <QUrl>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QFileDialog>

#include "resfile.h"

#include "gui/treewidget.hpp"

namespace
{
	typedef boost::shared_ptr<resfile> ResfilePtr;

	class ResfileItem: public QTreeWidgetItem
	{
	public:
		explicit ResfileItem(const ResfilePtr& res, int index = -1);

		void extract_files(const QString& path);

	private:
		enum Column
		{
			ColumnName,
			ColumnSize,
			ColumnModified
		};

	private:
		ResfilePtr res;
		int index;
	};
}

namespace ResfileViewer
{
	TreeWidget::TreeWidget(QWidget* parent):
		QTreeWidget(parent)
	{
		setAcceptDrops(true);
		setAllColumnsShowFocus(true);
		setAlternatingRowColors(true);
		setSortingEnabled(true);
		setSelectionMode(QAbstractItemView::ExtendedSelection);

		setHeaderLabels(QStringList("Name") << tr("Size") << tr("Modified"));
	}

	void TreeWidget::add_files(const QStringList& paths)
	{
		Q_FOREACH(const QString& path, paths) {
			ResfilePtr res(resfile_open_file(
				qPrintable(path)), resfile_close);
			if (res) {
				addTopLevelItem(new ResfileItem(res));
			}
		}
	}

	void TreeWidget::dragEnterEvent(QDragEnterEvent* event)
	{
		if (event->mimeData()->hasUrls()) {
			event->acceptProposedAction();
		}
	}

	void TreeWidget::dragMoveEvent(QDragMoveEvent* event)
	{
		if (event->mimeData()->hasUrls()) {
			event->acceptProposedAction();
		}
	}

	void TreeWidget::dropEvent(QDropEvent* event)
	{
		QList<QUrl> urls = event->mimeData()->urls();
		QStringList paths;
		std::transform(urls.constBegin(), urls.constEnd(),
					   std::back_inserter(paths),
					   std::mem_fun_ref(&QUrl::toLocalFile));
		add_files(paths);
	}

	void TreeWidget::contextMenuEvent(QContextMenuEvent* event)
	{
		QAction extract_action(tr("&Extract..."), this);
		extract_action.setDisabled(selectedItems().isEmpty());
		extract_action.setStatusTip(tr("Extract selected file(s)"));
		connect(&extract_action, SIGNAL(triggered()),
				this, SLOT(extract_files()));

		QMenu menu(this);
		menu.addAction(&extract_action);
		menu.exec(event->globalPos());
	}

	void TreeWidget::extract_files()
	{
		QString path = QFileDialog::getExistingDirectory(this,
								tr("Select output directory"));
		if (path.isEmpty()) {
			return;
		}

		Q_FOREACH(QTreeWidgetItem* item, selectedItems()) {
			static_cast<ResfileItem*>(item)->extract_files(path);
		}
	}
}

namespace
{
	ResfileItem::ResfileItem(const ResfilePtr& res, int index):
		QTreeWidgetItem(QTreeWidgetItem::UserType),
		res(res),
		index(index)
	{
		if (-1 == index) {
			QString name(resfile_name(res.get()));

			setText(ColumnName, name);
			setToolTip(ColumnName, name);

			for (int count = resfile_node_count(res.get()),
					index = 0; index < count; ++index) {
				addChild(new ResfileItem(res, index));
			}
		} else {
			QString name(resfile_node_name(index, res.get()));
			QString size(QString::number(resfile_node_size(index, res.get())));
			QString modified(QDateTime::fromTime_t(resfile_node_modified(
				index, res.get())).toString("dd.MM.yyyy hh:mm:ss"));

			setText(ColumnName, name);
			setToolTip(ColumnName, name);
			setText(ColumnSize, size);
			setToolTip(ColumnSize, size);
			setText(ColumnModified, modified);
			setToolTip(ColumnModified, modified);
		}
	}

	void ResfileItem::extract_files(const QString& path)
	{
		if (-1 == index) {
			for (int i = 0; i < childCount(); ++i) {
				static_cast<ResfileItem*>(child(i))->extract_files(path);
			}
		} else {
			/*QString res_name = QFileInfo(resfile_name(res.get())).baseName();
			QString node_name(resfile_node_name(index, res.get()));
			QString node_path = path + '/' + res_name + '/' +
								node_name.replace('\\', '/');
			QVector<char> buffer(resfile_node_size(index, res.get()));
			if (0 == resfile_node_data(buffer.data(), index, res.get())) {
				QDir().mkpath(QFileInfo(node_path).path());
				QFile file(node_path);
				if (file.open(QIODevice::WriteOnly)) {
					file.write(buffer.data(), buffer.size());
				}
			}*/
		}
	}
}
