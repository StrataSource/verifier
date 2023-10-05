//
// Created by ENDERZOMBI102 on 05/10/2023.
//

#pragma once

#include "ReportTableModel.hpp"
#include <QMainWindow>
#include <QLabel>
#include <QTableView>


#if defined(UNIX)
	#define EXEC_SUFFIX
	const auto DIVIDER = '/';
#else
	#define EXEC_SUFFIX ".exe"
	const auto DIVIDER = '\\';
#endif


class MainWindow : public QMainWindow {
	Q_OBJECT;
public:
	MainWindow();
private slots:
	void onExportReport( bool checked );
	void onGenerateManifest( bool checked );
	void onVerifyFiles( bool checked );
	void onExit( bool checked );
private:
	static QString getVerifierPath();
	static QStringList splitOutputLine( const QString& line );
private:
	QLabel* summaryLabel;
	QTableView* reportTable;
	ReportTableModel reportTableModel;
	QLineEdit* projectPath;
	QLineEdit* manifestPath;
};
