#include <QApplication>

#include "gui/mainwindow.hpp"

int main(int argc, char* argv[])
{
	QApplication application(argc, argv);

	ResfileViewer::MainWindow main_window;
	main_window.show();

	return application.exec();
}
