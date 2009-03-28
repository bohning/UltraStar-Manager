#ifndef QUSONGFILE_H_
#define QUSONGFILE_H_

#include <QObject>
#include <QFileInfo>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QDir>

#include "QU.h"
#include "QUSongLine.h"
#include "QUSongInterface.h"

/*!
 * This class represents a data file which is used by UltraStar for every song.
 * It contains all tags that are available in US Deluxe 1.00.
 */
class QUSongFile: public QUSongInterface {
	Q_OBJECT
    
public:
	QUSongFile(const QString &filePath, QObject *parent = 0);
	~QUSongFile();

    virtual void log(const QString &message, int type);

	bool hasUnsavedChanges() const { return _hasUnsavedChanges; }
	void setFile(const QString &filePath, bool update = true);

	bool updateCache();

	bool isValid();

	// sorting functions
	static bool artistLessThan (QUSongFile *s1, QUSongFile *s2);
	static bool titleLessThan (QUSongFile *s1, QUSongFile *s2);
	static bool languageLessThan (QUSongFile *s1, QUSongFile *s2);
	static bool editionLessThan (QUSongFile *s1, QUSongFile *s2);
	static bool genreLessThan (QUSongFile *s1, QUSongFile *s2);
	static bool yearLessThan (QUSongFile *s1, QUSongFile *s2);
	static bool creatorLessThan (QUSongFile *s1, QUSongFile *s2);

	static bool pathLessThan (QUSongFile *s1, QUSongFile *s2);
	static bool filePathLessThan (QUSongFile *s1, QUSongFile *s2);
	static bool relativeFilePathLessThan (QUSongFile *s1, QUSongFile *s2);

	static bool hasMp3LessThan (QUSongFile *s1, QUSongFile *s2);
	static bool hasCoverLessThan (QUSongFile *s1, QUSongFile *s2);
	static bool hasBackgroundLessThan (QUSongFile *s1, QUSongFile *s2);
	static bool hasVideoLessThan (QUSongFile *s1, QUSongFile *s2);

	// comparing functions
	static bool equal(QUSongFile *s1, QUSongFile *s2);

public slots:
	QString artist() const     {return _info.value(ARTIST_TAG,     QString(N_A));}
	QString title() const      {return _info.value(TITLE_TAG,      QString(N_A));}
	QString mp3() const        {return _info.value(MP3_TAG,        QString(N_A));}
	QString bpm() const        {return _info.value(BPM_TAG,        QString(N_A));}
	QString gap() const        {return _info.value(GAP_TAG,        QString(N_A));}
	QString video() const      {return _info.value(VIDEO_TAG,      QString(N_A));}
	QString videogap() const   {return _info.value(VIDEOGAP_TAG,   QString(N_A));}
	QString cover() const      {return _info.value(COVER_TAG,      QString(N_A));}
	QString background() const {return _info.value(BACKGROUND_TAG, QString(N_A));}
	QString start() const      {return _info.value(START_TAG,      QString(N_A));}
	QString language() const   {return _info.value(LANGUAGE_TAG,   QString(N_A));}
	QString relative() const   {return _info.value(RELATIVE_TAG,   QString(N_A));}
	QString edition() const    {return _info.value(EDITION_TAG,    QString(N_A));}
	QString genre() const      {return _info.value(GENRE_TAG,      QString(N_A));}
	QString year() const       {return _info.value(YEAR_TAG,       QString(N_A));}
	QString end() const        {return _info.value(END_TAG,        QString(N_A));}
	QString creator() const    {return _info.value(CREATOR_TAG,    QString(N_A));}

	QString customTag(const QString &tag) const {return _info.value(tag.toUpper(), QString(N_A));}

	QString dir() const {return _fi.dir().dirName();}
	QString path() const {return _fi.path();}
	QString filePath() const {return _fi.filePath();}
	QString relativeFilePath() const;
	QString txt() const {return _fi.fileName();}

	bool hasMp3() const;
	bool hasCover() const;
	bool hasBackground() const;
	bool hasVideo() const;

	bool isSongChecked() const; // for [SC]
	bool isSingStar() const;
	bool isDuet() const;
	bool isKaraoke() const;

	QString titleCompact() const;
	int length();
	int lengthMp3() const;
	int lengthEffective() const;
	int lengthAudioFile() const;
	double syllablesPerSecond(bool firstCalc = false);

	QString lengthEffectiveFormatted() const;
	QString speedFormatted();

	QStringList lyrics() const;

	QFileInfo songFileInfo() const { QFileInfo result(_fi); result.refresh(); return result; } //!< \returns a file info for the current US song file

	QFileInfo mp3FileInfo() const {return QFileInfo(_fi.dir(), mp3());} //!< \returns a file info for the mp3 file
	QFileInfo coverFileInfo() const {return QFileInfo(_fi.dir(), cover());} //!< \returns a file info for the cover file
	QFileInfo backgroundFileInfo() const {return QFileInfo(_fi.dir(), background());} //!< \returns a file info for the background file
	QFileInfo videoFileInfo() const {return QFileInfo(_fi.dir(), video());} //!< \returns a file info for the video file

	void setInfo(const QString &key, const QString &value);

	bool save(bool force = false);
	void renameSongDir(const QString &newName);
	void renameSongTxt(const QString &newName);
	void renameSongMp3(const QString &newName);
	void renameSongCover(const QString &newName);
	void renameSongBackground(const QString &newName);
	void renameSongVideo(const QString &newName);

	void useID3TagForArtist();
	void useID3TagForTitle();
	void useID3TagForGenre();
	void useID3TagForYear();

	static void verifyTags(QStringList &tags);

	bool unsupportedTagsFound() const { return _foundUnsupportedTags.size() > 0; }
	QString unsupportedTags() const { return _foundUnsupportedTags.join("\n#"); }
	void removeUnsupportedTags();

	void useExternalFile(const QString &filePath);
	void autoSetFiles();
	void autoSetFile(const QFileInfo &fi, bool force = false);

	void deleteUnusedFiles();
	void clearInvalidFileTags();

	void moveAllFiles(const QString &newRelativePath);

	void fixAudioLength();
	void roundGap();
	void removeEndTag();

	// watch external song file changes
	void songFileChanged(const QString &filePath);

	// for lyric operations
	virtual QList<QUSongLineInterface*>& loadMelody();
	virtual void clearMelody();
	virtual void saveMelody();
	QList<QUSongLineInterface*> melodyForSinger(QUSongLineInterface::Singers s = QUSongLineInterface::undefined);

	// Friends: Support for karaoke and duet song files
	void addFriend(QUSongFile *song);
	bool isFriend(const QFileInfo &fi);
	bool isFriend(const QString &fileName);
	bool isFriend(QUSongFile *song);
	QUSongFile* friendAt(const QFileInfo &fi);
	QUSongFile* friendAt(const QString &fileName);
	QList<QUSongFile*> friends() const { return _friends; }
	void changeData(const QString &tag, const QString &value);
	void renameSong(const QString &fileName);
	void changeSongPath(const QString &filePath);
	QUSongFile* duetFriend() const;
	bool friendHasTag(const QString &tag, const QString &value) const;

signals:
	void dataChanged(); // used to notify playlists for now
	void dataChanged(const QString &tag, const QString &value);
	void songRenamed(const QString &fileName);
	void songPathChanged(const QString &filePath);
	void externalSongFileChangeDetected(QUSongFile *song);

private:
	QFileInfo _fi;
	QStringList _fiTags; // list of original []-tags, e.g. "Wizo - Hund [karaoke] [blubb].txt"

	QMap<QString, QString> _info; // song header
	QStringList _lyrics;          // lyrics
	QStringList _footer;          // other stuff after the end mark 'E' in the song file

	QStringList _foundUnsupportedTags;
	bool _hasUnsavedChanges;

	int     _songLength; // cached song length, calculated from lyrics + gap, -1 == not calculated
	double  _songSpeed; // cached song speed

	bool rename(QDir dir, const QString &oldName, const QString &newName);

	// experimental: use another internal format for lyrics
	QList<QUSongLineInterface*> _melody;

	void convertLyricsFromRaw();
	void convertLyricsToRaw();
	void lyricsAddNote(QString line);

	QList<QUSongFile*> _friends; // karaoke or duet songs that share the same resources
};

#endif /*QUSONGFILE_H_*/
