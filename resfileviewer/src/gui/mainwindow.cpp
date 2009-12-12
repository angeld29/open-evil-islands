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
