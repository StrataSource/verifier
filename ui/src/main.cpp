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
	QApplication::setOrganizationName( "Strata Source" );
	QApplication::setApplicationVersion( VERIFIER_VERSION );

	auto window = new MainWindow();
	window->show();

	return QApplication::exec();
}
