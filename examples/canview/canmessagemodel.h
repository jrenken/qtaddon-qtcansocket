/*
 *  file:		canmessagemodel.h
 *  author:		jrenken
 *
 *  $Rev: 74 $
 *  $Author: jrenken $
 *  $Date: 2012-08-30 15:50:09 +0200 (Do, 30 Aug 2012) $
 *  $Id: canmessagemodel.h 74 2012-08-30 13:50:09Z jrenken $
 */

#ifndef CANMESSAGEMODEL_H_
#define CANMESSAGEMODEL_H_

#include <QAbstractItemModel>
#include <qcanmessage.h>

class CanMessageModel: public QAbstractTableModel
{
public:
	enum Type { Receive, Transmit } ;

	CanMessageModel(Type type = Receive, QObject *parent = 0);
	virtual ~CanMessageModel();

	void setData(const CanMessages & data);
	void addMessage(const QCanMessage& msg, bool count = true);
	void replaceMessage(const QCanMessage& msg, const QCanMessage& newMsg);
	void deleteMessage(const QCanMessage& msg);
	void deleteAll();
	void setShowTrigger(bool show);
	bool showTrigger() const {
		return mShowTrigger;
	}


	int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

private:
    Type 		mType;
    CanMessages	mMessages;
    bool mShowTrigger;
};

#endif /* CANMESSAGEMODEL_H_ */
