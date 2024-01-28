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
	const auto BIN_FOLDER = "linux64";
#else
	#define EXEC_SUFFIX ".exe"
	const auto DIVIDER = '\\';
	const auto BIN_FOLDER = "win64";
#endif


class MainWindow : public QMainWindow {
	Q_OBJECT;
public:
	MainWindow();
private slots:
	void onExportReport( bool checked );
	void onGenerateManifest( bool checked );
	void onVerifyFiles( bool checked );
private:
	static QString getVerifierPath();
	static QStringList splitOutputLine( const QString& line );
	void unlock();
	void lock();
private:
	QLabel* summaryLabel;
	QTableView* reportTable;
	ReportTableModel reportTableModel;
	QLineEdit* projectPath;
	QLineEdit* manifestPath;
	QLabel* statusLabel;
private:
	QAction* exportReportAction;
	QAction* generateManifestAction;
	QAction* verifyFilesAction;
};
