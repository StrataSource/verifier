//
// Created by ENDERZOMBI102 on 04/10/2023.
//
#include "MainWindow.hpp"

#include <QApplication>
#include <iostream>


int main( int argc, char* argv[] ) {
	QApplication app{ argc, argv };
	QApplication::setApplicationName( "verifier-ui" );
	QApplication::setApplicationDisplayName( "Verifier" );
	QApplication::setOrganizationName( "Strata" );
	QApplication::setApplicationVersion( "0.1.0" );

	auto window = new MainWindow();
	window->show();

	return QApplication::exec();
}
