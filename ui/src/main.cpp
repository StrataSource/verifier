//
// Created by ENDERZOMBI102 on 04/10/2023.
//
#include "main.hpp"

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QMenuBar>
#include <iostream>

int main( int argc, char* argv[] ) {
	QApplication app{ argc, argv };
	QApplication::setApplicationName( "verifier-ui" );
	QApplication::setApplicationVersion( "0.1.0" );

	auto window = new MainWindow();
	window->show();

	return QApplication::exec();
}

MainWindow::MainWindow() : QMainWindow() {
	this->setWindowTitle( tr( "Verifier" ) );
	this->setWindowIcon( QIcon( ":/icon.png" ) );
	this->setMinimumSize( 320, 240 );

	{// Build the menu bar
		auto fileMenu = this->menuBar()->addMenu( "File" );
		auto exportReport = new QAction( "Export Report" );
		connect( exportReport, &QAction::triggered, this, &MainWindow::onExportReport );
		fileMenu->addAction( exportReport );

		auto generateManifest = new QAction( "Generate Manifest" );
		connect( generateManifest, &QAction::triggered, this, &MainWindow::onGenerateManifest );
		fileMenu->addAction( generateManifest );

		fileMenu->addSeparator();
		auto exit = new QAction( "Exit" );
		connect( exit, &QAction::triggered, this, &MainWindow::onExit );
		fileMenu->addAction( exit );
	};
	{// Build the layout
		auto layout = new QBoxLayout( QBoxLayout::Direction::TopToBottom );

		this->setLayout( layout );
	}
}

void MainWindow::onExportReport( bool checked ) {
	std::cout << "onExportReport" << std::endl;
}

void MainWindow::onGenerateManifest( bool checked ) {
	std::cout << "onGenerateManifest" << std::endl;
}

void MainWindow::onExit( bool checked ) {
	QApplication::closeAllWindows();
}
