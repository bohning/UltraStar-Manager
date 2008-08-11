#ifndef QUPLAYLISTAREA_H_
#define QUPLAYLISTAREA_H_

#include "QU.h"
#include "QUSongFile.h"
#include "QUPlaylistFile.h"

#include <QString>
#include <QWidget>
#include <QList>

#include "ui_QUPlaylistArea.h"

class QUPlaylistArea: public QWidget, private Ui::QUPlaylistArea {
	Q_OBJECT

public:
	QUPlaylistArea(QWidget *parent = 0);
	~QUPlaylistArea();

public slots:
	void refreshAllPlaylists(QList<QUSongFile*> *songRef);
	void disconnectPlaylists();
	void update();

private slots:
	void createPlaylistFiles();
	void setCurrentPlaylist(int index);

	void updatePlaylistCombo();
	void updateCurrentPlaylistConnections();
	void updateCurrentPlaylistName(const QString &newName);

	void saveCurrentPlaylist();
	void saveCurrentPlaylistAs();

	void addPlaylist();
	void addPlaylist(const QString &filePath);

signals:
	void finished(const QString &message, QU::EventMessageTypes type);

private:
	QList<QUPlaylistFile*>  _playlists;
	QList<QUSongFile*>     *_songsRef;

	int currentPlaylistIndex(int index = -1) const;
};

#endif /* QUPLAYLISTAREA_H_ */
