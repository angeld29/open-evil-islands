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

#include <QtGlobal>
#include <QString>
#include <QStringList>
#include <QFileDialog>

#include "ui_mainwindow.h"

#include "gui/mainwindow.hpp"

namespace ResfileViewer
{
    MainWindow::MainWindow():
        ui(new Ui::MainWindow),
        tree_widget(this)
    {
        ui->setupUi(this);

        setCentralWidget(&tree_widget);

        connect(ui->action_open, SIGNAL(triggered()),
                this, SLOT(open_files()));
    }

    MainWindow::~MainWindow()
    {
    }

    void MainWindow::open_files()
    {
        QStringList paths = QFileDialog::getOpenFileNames(this,
                                tr("Open File(s)"), QString(),
                                tr("Res Files (*)"));
        if (!paths.isEmpty()) {
            tree_widget.add_files(paths);
        }
    }
}
