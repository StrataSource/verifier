//
// Created by ENDERZOMBI102 on 05/10/2023.
//

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QDialog>
#include <QFileDialog>
#include <QPushButton>
#include <QStatusBar>
#include <QLineEdit>
#include <QLabel>
#include <QMenuBar>
#include <QProcess>

#include "MainWindow.hpp"
#include "ReportTableModel.hpp"

MainWindow::MainWindow() : QMainWindow() {
	this->setWindowTitle( tr( "Verifier" ) );
	this->setWindowIcon( QIcon( ":/icon.png" ) );
	this->setMinimumSize( 640, 320 );
	this->statusBar()->showMessage( "Status: idle" );

	{// Build the menu bar
		auto fileMenu = this->menuBar()->addMenu( "File" );
		auto exportReport = new QAction( "Export Report", this );
		connect(exportReport, &QAction::triggered, this, &MainWindow::onExportReport );
		fileMenu->addAction( exportReport );

		auto generateManifest = new QAction( "Generate Manifest", this );
		connect(generateManifest, &QAction::triggered, this, &MainWindow::onGenerateManifest );
		fileMenu->addAction( generateManifest );

		auto verifyFiles = new QAction( "Verify Files", this );
		connect(verifyFiles, &QAction::triggered, this, &MainWindow::onVerifyFiles );
		fileMenu->addAction( verifyFiles );

		fileMenu->addSeparator();
		auto exit = new QAction( "Exit", this );
		connect(exit, &QAction::triggered, this, &MainWindow::onExit );
		fileMenu->addAction( exit );
	};
	{// Build the layout
		this->setCentralWidget( new QWidget( this ) );
		auto layout = new QBoxLayout( QBoxLayout::Direction::TopToBottom, this->centralWidget() );

		this->summaryLabel = new QLabel( "Discrepancies", this->centralWidget() );
		layout->addWidget( this->summaryLabel );
		this->reportTable = new QTableView( this->centralWidget() );
		this->reportTable->setModel( &this->reportTableModel );
		layout->addWidget( this->reportTable );

		{
			auto sublayout = new QBoxLayout( QBoxLayout::Direction::LeftToRight, this->centralWidget() );

			sublayout->addWidget( new QLabel( "Project  Path", this->centralWidget() ) );

			this->projectPath = new QLineEdit( this->centralWidget() );
			this->projectPath->setText( "C:/Program Files (x86)/Steam/steamapps/common/Portal 2 Community Edition" );
			sublayout->addWidget( this->projectPath );

			layout->addLayout( sublayout );
		};

		{
			auto sublayout = new QBoxLayout( QBoxLayout::Direction::LeftToRight, this->centralWidget() );

			sublayout->addWidget( new QLabel( "Manifest Path", this->centralWidget() ) );

			this->manifestPath = new QLineEdit( this->centralWidget() );
			this->manifestPath->setText( "default" );
			sublayout->addWidget( this->manifestPath );

			const auto button = new QPushButton( "Go!", this->centralWidget() );
			connect( button, &QPushButton::clicked, this, &MainWindow::onVerifyFiles );
			sublayout->addWidget( button );

			layout->addLayout( sublayout );
		}

		this->centralWidget()->setLayout( layout );
	}
	this->layout();
}

void MainWindow::onExportReport( bool checked ) {
	// ask where to save the report
	QFileDialog diag{ this };
	diag.setModal( true );
	diag.setFilter( QDir::Filter::Writable | QDir::Filter::Files );
	diag.setFileMode( QFileDialog::FileMode::AnyFile );
	diag.setDefaultSuffix( "index.csv" );
	diag.selectFile( "./index.csv" );
	diag.setNameFilter( "*.csv" );
	diag.setAcceptMode( QFileDialog::AcceptMode::AcceptSave );
	if ( diag.exec() != QDialog::DialogCode::Accepted )
		return;

	// save it
	//	for (  );
}

void MainWindow::onGenerateManifest( bool checked ) {
	auto proc = new QProcess( this );
	connect(
		proc, &QProcess::finished, this,
		[&](int exitCode, QProcess::ExitStatus exitStatus) -> void {
//			this->reportList->append( "finished(" + QString::number(exitCode) + ", " + QString::number(exitStatus) + ")" );
		}
	);

	connect(
		proc, &QProcess::readyReadStandardOutput, this,
		[=, this]() -> void {
//			this->reportList->append( "readyReadStandardOutput" );
//			this->reportList->append( proc->readAllStandardOutput() );
		}
	);
	proc->setProgram( getVerifierPath() );

	// Setup arguments
	QStringList arguments{ "--new-index", "-f", "csv" };
	if ( this->manifestPath->text() != "default" )
		arguments.append( { "-i", this->manifestPath->text() } );
	proc->setArguments( arguments );

	// Add the VPROJECT environment variable
	auto env = QProcessEnvironment::systemEnvironment();
	env.insert( "VPROJECT", this->projectPath->text() );
	proc->setProcessEnvironment(env);

	proc->start();
}

void MainWindow::onVerifyFiles( bool checked ) {
	this->reportTableModel.clear();
	const auto proc = new QProcess( this );

	connect(
		proc, &QProcess::finished, this,
		[&](int exitCode, QProcess::ExitStatus exitStatus) -> void {
			//			this->reportList->append( "finished(" + QString::number(exitCode) + ", " + QString::number(exitStatus) + ")" );
		}
	);
	connect(
		proc, &QProcess::readyReadStandardError, this,
		[=, this]() -> void {
			this->statusBar()->showMessage( proc->readAllStandardError() );
		}
	);
	connect(
		proc, &QProcess::readyReadStandardOutput, this,
		[=, this]() -> void {
			const auto line = proc->readAllStandardOutput();

			// skip header
			if ( line.startsWith("ty") )
				return;

			const auto parts = splitOutputLine( line );

			// reports should be put in the table, while messages in the status bar
			if ( parts[0] == "report" )
				this->reportTableModel.pushReport( parts[1], parts[3], parts[4] );
			else
				this->statusBar()->showMessage( parts[2] );
		}
	);
	proc->setProgram( getVerifierPath() );

	// Setup arguments
	QStringList arguments{ "-f", "csv" };
	if ( this->manifestPath->text() != "default" )
		arguments.append( { "-i", this->manifestPath->text() } );
	proc->setArguments( arguments );

	// Add the VPROJECT environment variable
	auto env = QProcessEnvironment::systemEnvironment();
	env.insert( "VPROJECT", this->projectPath->text() );
	proc->setProcessEnvironment(env);

	proc->start();
}

void MainWindow::onExit( bool checked ) {
	QApplication::closeAllWindows();
}

QString MainWindow::getVerifierPath() {
	auto folder = QApplication::arguments()[0];
	folder.resize( folder.lastIndexOf( DIVIDER ) );
	auto verifierExe = folder + "/verifier" EXEC_SUFFIX;

	// does this exist?
	if ( QFile::exists( verifierExe ) )
		return verifierExe;

	// try with current folder
	if ( QFile::exists( "./verifier" EXEC_SUFFIX ) )
		return "./verifier" EXEC_SUFFIX;

	// found nothing...
	return "";
}

QStringList MainWindow::splitOutputLine( const QString& line ) {
	QStringList parts{3};
	qsizetype lastPos = 1;
	while ( true ) {
		auto pos = line.indexOf( "\",", lastPos );
		if ( pos == line.size() || pos == -1 ) {
			pos = lastPos + 2;
			parts.append( line.sliced( pos, line.size() - pos ) );
			break;
		}
		parts.append( line.sliced( lastPos, pos - lastPos ).replace( ",", "" ) );
		lastPos = pos + 1;
	}

	return parts;
}
