//
// Created by ENDERZOMBI102 on 05/10/2023.
//

#pragma once


#include <QAbstractTableModel>

class ReportTableModel : public QAbstractTableModel {
private:
	struct Report {
		QString filename;
		QString found;
		QString expected;
	};
public:
public:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	void pushReport( QString filename, QString found, QString expected );
private:
	QList<Report> reports;
};
