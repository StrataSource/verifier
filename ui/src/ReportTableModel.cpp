//
// Created by ENDERZOMBI102 on 05/10/2023.
//

#include "ReportTableModel.hpp"

#include <utility>


int ReportTableModel::rowCount( const QModelIndex& parent ) const {
	return static_cast<int>( this->reports.count() );
}

QVariant ReportTableModel::data( const QModelIndex& index, int role ) const {
	if ( index.row() > this->reports.count() || role != Qt::ItemDataRole::DisplayRole || !index.isValid() )
		return {};

	auto& report = this->reports[index.row()];
	switch ( index.column() ) {
		case 0: return report.filename;
		case 1: return report.found;
		case 2: return report.expected;
	}

	return {};
}

int ReportTableModel::columnCount( const QModelIndex& parent ) const {
	return 3;
}

QVariant ReportTableModel::headerData( int section, Qt::Orientation orientation, int role ) const {
	if ( role != Qt::DisplayRole || orientation != Qt::Orientation::Horizontal )
		return {};

	switch ( section ) {
		case 0: return "File name";
		case 1: return "Found";
		case 2: return "Expected";
		default: return {};
	}
}

void ReportTableModel::pushReport( QString filename, QString found, QString expected ) {
	const auto newRow = static_cast<int>( this->reports.size() );
	beginInsertRows( QModelIndex(), newRow, newRow );

		this->reports.append({ std::move(filename), std::move(found), std::move(expected) });

	endInsertRows();
}
void ReportTableModel::clear() {
	beginRemoveRows( QModelIndex(), 0, static_cast<int>( this->reports.size() ) );

		this->reports.clear();

	endInsertRows();
}
