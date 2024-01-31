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

MainWindow::MainWindow() : QMainWindow() {
	this->setWindowTitle( tr( "Verifier" ) );
	this->setWindowIcon( QIcon( ":/icon.png" ) );
	this->setMinimumSize( 640, 320 );
	this->statusLabel = new QLabel( tr( "Status: idle" ), this );
	this->statusBar()->addPermanentWidget( this->statusLabel );

	{// Build the menu bar
		auto fileMenu = this->menuBar()->addMenu( tr( "File" ) );
		this->exportReportAction = new QAction( tr( "Export Report" ), this );
		connect( exportReportAction, &QAction::triggered, this, &MainWindow::onExportReport );
		fileMenu->addAction( exportReportAction );

		this->generateManifestAction = new QAction( tr( "Generate Manifest" ), this );
		connect( generateManifestAction, &QAction::triggered, this, &MainWindow::onGenerateManifest );
		fileMenu->addAction( generateManifestAction );

		this->verifyFilesAction = new QAction( tr( "Verify Files" ), this );
		connect( verifyFilesAction, &QAction::triggered, this, &MainWindow::onVerifyFiles );
		fileMenu->addAction( verifyFilesAction );

		fileMenu->addSeparator();
		auto exit = new QAction( tr( "Exit" ), this );
		connect(exit, &QAction::triggered, this, [](bool checked) -> void { QApplication::closeAllWindows(); } );
		fileMenu->addAction( exit );
	};
	{// Build the layout
		this->setCentralWidget( new QWidget( this ) );
		auto layout = new QBoxLayout( QBoxLayout::Direction::TopToBottom, this->centralWidget() );

		this->summaryLabel = new QLabel( tr( "Differences" ), this->centralWidget() );
		layout->addWidget( this->summaryLabel );
		this->reportTable = new QTableView( this->centralWidget() );
		this->reportTable->setModel( &this->reportTableModel );
		layout->addWidget( this->reportTable );

		{
			auto sublayout = new QBoxLayout( QBoxLayout::Direction::LeftToRight, this->centralWidget() );

			sublayout->addWidget( new QLabel( tr( "Project Path" ), this->centralWidget() ) );

			auto exeDirPath = QApplication::applicationDirPath();

			#if defined( DEBUG )
				auto path = QString( "/drive/SteamLibrary/steamapps/common/Portal 2 Community Edition/" );
			#else
				auto path = QString{};
			#endif

			if ( exeDirPath.endsWith( BIN_FOLDER ) )
				path = QString::fromStdWString( std::filesystem::path( exeDirPath.toStdWString() )
					.parent_path() // win64 -> bin
					.parent_path() // bin -> root
					.generic_wstring()
				);

			this->projectPath = new QLineEdit( this->centralWidget() );
			this->projectPath->setText( path );
			sublayout->addWidget( this->projectPath );

			layout->addLayout( sublayout );
		};

		{
			auto sublayout = new QBoxLayout( QBoxLayout::Direction::LeftToRight, this->centralWidget() );

			sublayout->addWidget( new QLabel( tr( "Manifest Path" ), this->centralWidget() ) );

			this->manifestPath = new QLineEdit( this->centralWidget() );
			this->manifestPath->setText( "default" );
			sublayout->addWidget( this->manifestPath );

			const auto button = new QPushButton( tr( "Go!" ), this->centralWidget() );
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
	diag.setDefaultSuffix( "index.rsv" );
	diag.selectFile( "./index.rsv" );
	diag.setNameFilter( "*.rsv" );
	diag.setAcceptMode( QFileDialog::AcceptMode::AcceptSave );
	if ( diag.exec() != QDialog::DialogCode::Accepted )
		return;

	// save it
	//	for (  );
}

void MainWindow::onGenerateManifest( bool checked ) {
	this->lock();
	auto proc = new QProcess( this );
	connect(
		proc, &QProcess::finished, this,
		[&](int exitCode, QProcess::ExitStatus exitStatus) -> void {
			this->statusLabel->setText( tr( "Status: finished (exit code %1)" ).arg( exitCode ) );
			this->unlock();
		}
	);
	connect(
		proc, &QProcess::readyReadStandardError, this,
		[=, this]() -> void { this->statusLabel->setText( proc->readAllStandardError() ); }
	);
	connect(
		proc, &QProcess::readyReadStandardOutput, this,
		[&]() -> void {
//			this->reportList->append( "readyReadStandardOutput" );
//			this->reportList->append( proc->readAllStandardOutput() );
		}
	);
	proc->setProgram( getVerifierPath() );

	// Setup arguments
	QStringList arguments{ "--new-index", "-f", "csv", "--root", this->projectPath->text() };
	if ( this->manifestPath->text() != "default" )
		arguments.append( { "-i", this->manifestPath->text() } );
	proc->setArguments( arguments );

	proc->start();
}

void MainWindow::onVerifyFiles( bool checked ) {
	if ( this->projectPath->text().isEmpty() ) {
		// TODO: Add message box with error
	}
	// TODO: Ditto as above but for manifestPath

	this->reportTableModel.clear();
	this->lock();
	const auto proc = new QProcess( this );

	connect(
		proc, &QProcess::finished, this,
		[=, this](int exitCode, QProcess::ExitStatus exitStatus) -> void {
			this->statusLabel->setText( tr( "Status: finished (exit code %1)" ).arg( exitCode ) );
			this->unlock();
		}
	);
	connect(
		proc, &QProcess::readyReadStandardError, this,
		[=, this]() -> void {
			this->statusLabel->setText( proc->readAllStandardError() );
		}
	);
	connect(
		proc, &QProcess::readyReadStandardOutput, this,
		[=, this]() -> void {
			const auto lines = proc->readAllStandardOutput();

			for ( const auto& line : lines.split('\n') ) {
				// skip header
				if ( line.startsWith( "ty" ) || line.isEmpty() )
					continue;

				// `[type,context,message,got?,expected?]` where `got` and `expected` are present if `type` is `report`
				const auto parts = splitOutputLine( line );
				qDebug() << parts;

				// reports should be put in the table, while messages in the status bar
				if ( parts[ 0 ] == "report" )
					this->reportTableModel.pushReport( parts[ 1 ], parts[ 3 ], parts[ 4 ] );
				else
					this->statusLabel->setText( parts[ 2 ] );
			}
		}
	);


	connect( proc, &QProcess::errorOccurred, this, [&proc](QProcess::ProcessError error) {
		qDebug( "%u %u", error, proc->exitCode() );
	});
	proc->setProgram( getVerifierPath() );

	// Setup arguments
	QStringList arguments{ "-f", "csv", "--root", this->projectPath->text() };
	if ( this->manifestPath->text() != "default" )
		arguments.append( { "-i", this->manifestPath->text() } );
	proc->setArguments( arguments );

	proc->start();
}

QString MainWindow::getVerifierPath() {
	auto verifierExe = QApplication::applicationDirPath() + "/verifier" EXEC_SUFFIX;

	// does this exist?
	if ( QFile::exists( verifierExe ) )
		return verifierExe;

	// try with current folder
	if ( QFile::exists( "./verifier" EXEC_SUFFIX ) )
		return "./verifier" EXEC_SUFFIX;

	// found nothing...
	return {};
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

void MainWindow::lock() {
	this->exportReportAction->setEnabled( false );
	this->generateManifestAction->setEnabled( false );
	this->verifyFilesAction->setEnabled( false );
}

void MainWindow::unlock() {
	this->exportReportAction->setEnabled( true );
	this->generateManifestAction->setEnabled( true );
	this->verifyFilesAction->setEnabled( true );
}
