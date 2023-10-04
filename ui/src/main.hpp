//
// Created by ENDERZOMBI102 on 04/10/2023.
//
#pragma once

#include <QApplication>
#include <QMainWindow>

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow();
public:
	void onExportReport( bool checked );
	void onGenerateManifest( bool checked );
	void onExit( bool checked );
}
