#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QSharedPointer>
#include <QMainWindow>

#include "gui/treewidget.hpp"

namespace Ui
{
	class MainWindow;
}

namespace ResfileViewer
{
	class MainWindow: public QMainWindow
	{
		Q_OBJECT

	public:
		MainWindow();
		~MainWindow();

	private Q_SLOTS:
		void open_files();

	private:
		QSharedPointer<Ui::MainWindow> ui;
		TreeWidget tree_widget;
	};
}

#endif /* MAINWINDOW_HPP */
