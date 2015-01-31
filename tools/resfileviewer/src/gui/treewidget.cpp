/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2015 Yanis Kurganov <ykurganov@users.sourceforge.net>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

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

#define restrict
#include <cealloc.h>
#include <ceresfile.h>

#include "gui/treewidget.hpp"

namespace
{
    typedef boost::shared_ptr<ce_res_file> ResfilePtr;

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

        ce_alloc_init();
    }

    void TreeWidget::add_files(const QStringList& paths)
    {
        Q_FOREACH(const QString& path, paths) {
            ResfilePtr res(ce_res_file_new_path(
                qPrintable(path)), ce_res_file_del);
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
            QString name(res->name->str);

            setText(ColumnName, name);
            setToolTip(ColumnName, name);

            for (unsigned int count = res->node_count, index = 0; index < count; ++index) {
                addChild(new ResfileItem(res, index));
            }
        } else {
            QString name(ce_res_file_node_name(res.get(), index));
            QString size(QString::number(ce_res_file_node_size(res.get(), index)));
            QString modified(QDateTime::fromTime_t(ce_res_file_node_modified(res.get(), index)).toString("dd.MM.yyyy hh:mm:ss"));

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
