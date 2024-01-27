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
	void pushReport( QString filename, QString found, QString expected );
	void clear();
public:
	int rowCount(const QModelIndex &parent) const override;
	int columnCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
private:
	QList<Report> reports;
};
