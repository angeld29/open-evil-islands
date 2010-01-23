/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
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
