#include "QUMainWindow.h"
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <QListWidget>
#include <QListWidgetItem>

#include <QDir>
#include <QStringList>
#include <QPixmap>
#include <QIcon>
#include <QBrush>
#include <QHeaderView>
#include <QDateTime>
#include <QSettings>
#include <QMessageBox>
#include <QKeySequence>

#include <QFileDialog>
#include <QFileInfo>
#include <QProgressDialog>

#include "QUSongFile.h"
#include "QUDetailItem.h"
#include "QUMonty.h"

#include "QUTagOrderDialog.h"

QUMainWindow::QUMainWindow(QWidget *parent): QMainWindow(parent) {
	setupUi(this);
	
	initWindow();
	initMenu();
	initConfig();
	
	initSongTree();
	initDetailsTable();	
	initTaskList();
	
	initMonty();
}

/*!
 * Save configuration for next application start.
 */
QUMainWindow::~QUMainWindow() {
	QSettings settings;
	
	settings.setValue("showTaskList", QVariant(actionShowTaskList->isChecked()));
	settings.setValue("showEventLog", QVariant(actionShowEventLog->isChecked()));
	settings.setValue("allowMonty", QVariant(actionAllowMonty->isChecked()));
}

/*!
 * Initializes the windows registry entry for uman. Lets the user
 * choose a path where the song files are located.
 */
void QUMainWindow::initConfig() {
	QCoreApplication::setOrganizationName("HPI");
	QCoreApplication::setApplicationName("UltraStar Manager");
	     
	QSettings settings;
	QString path = settings.value("songPath").toString();
	
	path = QFileDialog::getExistingDirectory(this, "Choose your UltraStar song directory", path);
	
	if(!path.isEmpty())
		settings.setValue("songPath", QVariant(path));
	
	_baseDir.setPath(path);
	
	// read other settings
	actionShowTaskList->setChecked(settings.value("showTaskList", QVariant(true)).toBool());
	actionShowEventLog->setChecked(settings.value("showEventLog", QVariant(true)).toBool());
	actionAllowMonty->setChecked(settings.value("allowMonty", QVariant(true)).toBool());
}

/*!
 * Set up initial window size and title text.
 */
void QUMainWindow::initWindow() {
	setWindowTitle("UltraStar Manager");
	resize(1000, 600);
	QList<int> sizes;
	sizes << 700 << 300;
	splitter->setSizes(sizes);
}

void QUMainWindow::initMenu() {
	// song
	connect(actionExpandAll, SIGNAL(triggered()), songTree, SLOT(expandAll()));
	connect(actionExpandAll, SIGNAL(triggered()), this, SLOT(resizeToContents()));
	connect(actionCollapseAll, SIGNAL(triggered()), songTree, SLOT(collapseAll()));
	connect(actionCollapseAll, SIGNAL(triggered()), this, SLOT(resizeToContents()));
	connect(actionRefresh, SIGNAL(triggered()), this, SLOT(refreshSongs()));
	
	actionRefresh->setShortcut(QKeySequence::fromString("F5"));
	
	//options
	connect(actionShowEventLog, SIGNAL(toggled(bool)), log, SLOT(setVisible(bool)));
	connect(actionShowTaskList, SIGNAL(toggled(bool)), taskFrame, SLOT(setVisible(bool)));
	connect(actionTagSaveOrder, SIGNAL(triggered()), this, SLOT(editTagOrder()));
	
	// help
	connect(actionShowMonty, SIGNAL(triggered()), helpFrame, SLOT(show()));
	connect(actionQt, SIGNAL(triggered()), this, SLOT(aboutQt()));
	connect(actionUman, SIGNAL(triggered()), this, SLOT(aboutUman()));
	
	actionAllowMonty->setIcon(QIcon(":/monty/normal.png"));
	actionShowMonty->setIcon(QIcon(":/monty/happy.png"));
}

/*!
 * Set up the song tree the first time.
 * \sa createSongFiles()
 * \sa createSongTree()
 */
void QUMainWindow::initSongTree() {
	initSongTreeHeader();
	
	connect(songTree, SIGNAL(itemSelectionChanged()), this, SLOT(updateDetails()));

	connect(songTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(resetLink(QTreeWidgetItem*, int))); 
	connect(songTree, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(resizeToContents()));
	connect(songTree, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(resizeToContents()));
	
	refreshSongs();
}

void QUMainWindow::initSongTreeHeader() {
	QTreeWidgetItem *header = new QTreeWidgetItem();
	header->setText(0, "Folder");
	header->setIcon(0, QIcon(":/types/folder.png"));
	header->setText(1, "Artist");
	header->setIcon(1, QIcon(":/types/user.png"));
	header->setToolTip(1, "Shows whether your folder includes the artist correctly");
	header->setText(2, "Title");
	header->setIcon(2, QIcon(":/types/font.png"));
	header->setToolTip(2, "Shows whether your folder includes the title correctly");
	header->setText(3, "Audio");
	header->setIcon(3, QIcon(":/types/music.png"));
	header->setToolTip(3, "Shows whether the song text file points to an audio file that can be found by UltraStar");
	header->setText(4, "Cover");
	header->setIcon(4, QIcon(":/types/picture.png"));
	header->setToolTip(4, "Shows whether the song text file points to a picture that can be found by UltraStar");
	header->setText(5, "Background");
	header->setIcon(5, QIcon(":/types/picture.png"));
	header->setToolTip(5, "Shows whether the song text file points to a picture that can be found by UltraStar");
	header->setText(6, "Video");
	header->setIcon(6, QIcon(":/types/film.png"));
	header->setToolTip(6, "Shows whether the song text file points to a video file that can be found by UltraStar");
	
	songTree->setHeaderItem(header);	
}

void QUMainWindow::initDetailsTable() {
	detailsTable->verticalHeader()->hide();
	detailsTable->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	detailsTable->setHorizontalHeaderLabels(QStringList() << "Tag" << "Value");
	
	detailsTable->horizontalHeader()->setResizeMode(0, QHeaderView::Interactive);
	detailsTable->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
	
	connect(detailsTable, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(saveSongChanges(QTableWidgetItem*)));
}

void QUMainWindow::initTaskList() {
	QStringList tasks;
	tasks << "Get artist and title from ID3 tag";
	tasks << "Rename directory to \"Artist - Title\"";
	tasks << "Rename directory to \"Artist - Title [VIDEO] [SC]\" if checked or video present";
	tasks << "Rename songtext file to \"Artist - Title.txt\"";
	tasks << "Rename audio file to \"Artist - Title.*\"";
	tasks << "Rename cover to \"Artist - Title [CO].*\"";
	tasks << "Rename background to \"Artist - Title [BG].*\"";
	tasks << "Rename video to \"Artist - Title.*\"";
	tasks << "Rename video to \"Artist - Title [VD#*].*\" consider VIDEOGAP";
	
	taskList->addItems(tasks);
	
	for(int i = 0; i < taskList->count(); i++) {
		taskList->item(i)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
		taskList->item(i)->setCheckState(Qt::Unchecked);
	}
	
	taskList->item(0)->setIcon(QIcon(":/marks/tag.png"));
	taskList->item(1)->setIcon(QIcon(":/types/folder.png"));
	taskList->item(2)->setIcon(QIcon(":/types/folder.png"));
	taskList->item(3)->setIcon(QIcon(":/types/text.png"));
	taskList->item(4)->setIcon(QIcon(":/types/music.png"));
	taskList->item(5)->setIcon(QIcon(":/types/picture.png"));
	taskList->item(6)->setIcon(QIcon(":/types/picture.png"));
	taskList->item(7)->setIcon(QIcon(":/types/film.png"));
	taskList->item(8)->setIcon(QIcon(":/types/film.png"));
	
	connect(taskBtn, SIGNAL(clicked()), this, SLOT(doTasks()));
	connect(allTasksBtn, SIGNAL(clicked()), this, SLOT(checkAllTasks()));
	connect(noTasksBtn, SIGNAL(clicked()), this, SLOT(uncheckAllTasks()));
}

void QUMainWindow::initMonty() {
	montyLbl->setPixmap(monty->pic(QUMonty::seated));
	helpLbl->setText(monty->welcomeMsg(_songs.size()));
	
	connect(hideMontyBtn, SIGNAL(clicked()), helpFrame, SLOT(hide()));
	
	if(!actionAllowMonty->isChecked())
		helpFrame->hide();
}

/*!
 * Re-reads all possible song files and builds a new song tree.
 */
void QUMainWindow::refreshSongs() {
	songTree->clear();
	updateDetails();
	qDeleteAll(_songs);
	_songs.clear();
	
	createSongFiles();
	createSongTree();
}

/*!
 * Creates all instances of QUSongFile to fill the song tree.
 * \sa CreateSongTree();
 */
void QUMainWindow::createSongFiles() {
	QStringList entries = _baseDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::Name);
	QProgressDialog progress("Reading song files...", 0, 0, entries.size(), this);
	
	progress.show();
	
	foreach(QString sub, entries) {
		progress.setValue(progress.value() + 1);
		
		QDir subDir(_baseDir.path() + "/" + sub);
		QStringList files = subDir.entryList(QStringList("*.txt"), QDir::Files);
		
		if(!files.isEmpty()) {
			_songs.append(new QUSongFile(subDir.path() + "/" + files.first()));
		}
	}
}

/*!
 * Creates the song tree and enables the file system watcher for
 * the entries.
 */
void QUMainWindow::createSongTree() {
	foreach(QUSongFile* song, _songs) {
		QUSongItem *top = new QUSongItem(song, true);
		songTree->addTopLevelItem(top);
	}
	
	resizeToContents();
	songTree->sortItems(0, Qt::AscendingOrder);
}

void QUMainWindow::updateImage() {
//	QUSongItem *songItem = dynamic_cast<QUSongItem*>(songTable->currentItem());
//	
//	if(!songItem)
//		return;
//	
//	if(songTable->currentColumn() == 4) { // show cover
//		imageLbl->setPixmap(QPixmap(songItem->song()->coverFileInfo().filePath()));		
//	} else if(songTable->currentColumn() == 5) { // show background
//		imageLbl->setPixmap(QPixmap(songItem->song()->backgroundFileInfo().filePath()));		
//	}
}

void QUMainWindow::resizeToContents() {
	for(int i = 0; i < songTree->columnCount(); i++)
		songTree->resizeColumnToContents(i);
}

void QUMainWindow::resetLink(QTreeWidgetItem *item, int column) {
	QUSongItem *songItem = dynamic_cast<QUSongItem*>(item);
	
	if(!songItem || songItem->isToplevel())
		return;
	
	QUSongFile *song = songItem->song();
	
	QString fileScheme("*." + QFileInfo(songItem->text(0)).suffix());
	
	if( songItem->icon(3).isNull() 
			and QUSongFile::allowedAudioFiles().contains(fileScheme, Qt::CaseInsensitive) 
			and column == 3 ) {
		song->setInfo("MP3", songItem->text(0));
		song->save();
	} else if( songItem->icon(4).isNull() 
			and QUSongFile::allowedPictureFiles().contains(fileScheme, Qt::CaseInsensitive) 
			and column == 4 ) {
		song->setInfo("COVER", songItem->text(0));
		song->save();
	} else if( songItem->icon(5).isNull() 
			and QUSongFile::allowedPictureFiles().contains(fileScheme, Qt::CaseInsensitive) 
			and column == 5 ) {
		song->setInfo("BACKGROUND", songItem->text(0));
		song->save();
	} else if( songItem->icon(6).isNull() 
			and QUSongFile::allowedVideoFiles().contains(fileScheme, Qt::CaseInsensitive) 
			and column == 6 ) {
		song->setInfo("VIDEO", songItem->text(0));
		song->save();
	}

	(dynamic_cast<QUSongItem*>(songItem->parent()))->update();
	updateDetails();
	
	montyTalk();
}

void QUMainWindow::updateDetails() {
	QUSongItem *songItem = dynamic_cast<QUSongItem*>(songTree->currentItem());
	
	detailsTable->clearContents();
	
	if(!songItem)
		return;
	
	QUSongFile *song = songItem->song();
	
	detailsTable->setItem(0, 0, new QTableWidgetItem(QIcon(":/types/user.png"), "Artist"));
	detailsTable->setItem(1, 0, new QTableWidgetItem(QIcon(":/types/font.png"), "Title"));
	detailsTable->setItem(2, 0, new QTableWidgetItem(QIcon(":/types/music.png"), "MP3"));
	detailsTable->setItem(3, 0, new QTableWidgetItem(QIcon(":/types/picture.png"), "Cover"));
	detailsTable->setItem(4, 0, new QTableWidgetItem(QIcon(":/types/picture.png"), "Background"));
	detailsTable->setItem(5, 0, new QTableWidgetItem(QIcon(":/types/film.png"), "Video"));
	
	for(int i = 0; i < 6; i++)
		detailsTable->item(i, 0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	
	detailsTable->setItem(0, 1, new QUDetailItem(song->artist(), "ARTIST", song));
	detailsTable->setItem(1, 1, new QUDetailItem(song->title(), "TITLE", song));
	detailsTable->setItem(2, 1, new QUDetailItem(song->mp3(), "MP3", song));
	detailsTable->setItem(3, 1, new QUDetailItem(song->cover(), "COVER", song));
	detailsTable->setItem(4, 1, new QUDetailItem(song->background(), "BACKGROUND", song));
	detailsTable->setItem(5, 1, new QUDetailItem(song->video(), "VIDEO", song));

	for(int i = 0; i < 6; i++)
		detailsTable->item(i, 1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	
	// other song details, not editable
	detailsTable->setItem(6, 0, new QTableWidgetItem(QIcon(":/bullets/bullet_black.png"), "BPM"));
	detailsTable->setItem(7, 0, new QTableWidgetItem(QIcon(":/bullets/bullet_black.png"), "Gap"));
	detailsTable->setItem(8, 0, new QTableWidgetItem(QIcon(":/bullets/bullet_black.png"), "Start"));
	detailsTable->setItem(9, 0, new QTableWidgetItem(QIcon(":/bullets/bullet_black.png"), "Videogap"));
	detailsTable->setItem(10, 0, new QTableWidgetItem(QIcon(":/bullets/bullet_black.png"), "Relative"));
	detailsTable->setItem(11, 0, new QTableWidgetItem(QIcon(":/bullets/bullet_black.png"), "Language"));
	detailsTable->setItem(12, 0, new QTableWidgetItem(QIcon(":/bullets/bullet_black.png"), "Genre"));
	detailsTable->setItem(13, 0, new QTableWidgetItem(QIcon(":/bullets/bullet_black.png"), "Edition"));

	detailsTable->setItem(6, 1, new QTableWidgetItem(song->bpm()));
	detailsTable->setItem(7, 1, new QTableWidgetItem(QString("%1 milliseconds").arg(song->gap())));
	detailsTable->setItem(8, 1, new QTableWidgetItem(QString("%1 seconds").arg(song->start())));
	detailsTable->setItem(9, 1, new QTableWidgetItem(QString("%1 seconds").arg(song->videogap())));
	detailsTable->setItem(10, 1, new QTableWidgetItem(song->relative()));
	detailsTable->setItem(11, 1, new QTableWidgetItem(song->language()));
	detailsTable->setItem(12, 1, new QTableWidgetItem(song->genre()));
	detailsTable->setItem(13, 1, new QTableWidgetItem(song->edition()));
	
	for(int i = 6; i < 14; i++) {
		detailsTable->item(i, 0)->setFlags(Qt::ItemIsEnabled);
		detailsTable->item(i, 1)->setFlags(0);
	}
}

void QUMainWindow::saveSongChanges(QTableWidgetItem *item) {
	QUDetailItem *detailItem = dynamic_cast<QUDetailItem*>(detailsTable->currentItem());
	QUSongItem *songItem = dynamic_cast<QUSongItem*>(songTree->currentItem());
	
	if(!detailItem || !songItem)
		return;
	
	QUSongFile *song = detailItem->song();
	
	song->setInfo(detailItem->tag(), detailItem->text());
	song->save();
	
	songItem->update();
	
	updateDetails(); // to show "n/a" if text was deleted completely
}

void QUMainWindow::checkAllTasks() {
	for(int i = 0; i < taskList->count(); i++)
		taskList->item(i)->setCheckState(Qt::Checked);
}

void QUMainWindow::uncheckAllTasks() {
	for(int i = 0; i < taskList->count(); i++)
		taskList->item(i)->setCheckState(Qt::Unchecked);	
}

/*!
 * Does all checked tasks for all selected songs.
 * \sa initTaskList();
 */
void QUMainWindow::doTasks() {
	foreach(QTreeWidgetItem *item, songTree->selectedItems()) {
		QUSongItem *songItem = dynamic_cast<QUSongItem*>(item);

		if(songItem) {	
			QUSongFile *song = songItem->song();

			if(taskList->item(0)->checkState() == Qt::Checked)
				useID3Tag(song);
			if(taskList->item(1)->checkState() == Qt::Checked)
				renameSongDir(song);
			if(taskList->item(2)->checkState() == Qt::Checked)
				renameSongDirCheckedVideo(song);
			if(taskList->item(3)->checkState() == Qt::Checked)
				renameSongTxt(song);
			if(taskList->item(4)->checkState() == Qt::Checked)
				renameSongMp3(song);
			if(taskList->item(5)->checkState() == Qt::Checked)
				renameSongCover(song);
			if(taskList->item(6)->checkState() == Qt::Checked)
				renameSongBackground(song);
			if(taskList->item(7)->checkState() == Qt::Checked)
				renameSongVideo(song);
			if(taskList->item(8)->checkState() == Qt::Checked)
				renameSongVideogap(song);

			song->save();
			songItem->update();
		}
	}

	updateDetails();
	montyTalk();
}

void QUMainWindow::useID3Tag(QUSongFile *song) {
	QString done1("ID3Tag used for artist. Changed from: \"%1\" to:  \"%2\".");
	QString done2("ID3Tag used for title. Changed from: \"%1\" to: \"%2\".");
	QString fail("ID3Tag could NOT be used for artist and title.");
	
	QString oldArtist(song->artist());
	QString oldTitle(song->title());
	
	if(song->useID3Tag()) {
		addLogMsg(done1.arg(oldArtist).arg(song->artist()));
		addLogMsg(done2.arg(oldTitle).arg(song->title()));
	} else {
		addLogMsg(fail, 1);
	}
}

void QUMainWindow::renameSongDir(QUSongFile *song) {
	QString done("Song directory renamed from: \"%1\" to: \"%2\".");
	QString fail("Song directory: \"%1\" was NOT renamed.");
	
	QString oldName(song->songFileInfo().dir().dirName());
	
	if(song->renameSongDir(song->artist() + " - " + song->title())) {
		addLogMsg(done.arg(oldName).arg(song->songFileInfo().dir().dirName()));
	} else {
		addLogMsg(fail.arg(oldName), 1);
	}
}

void QUMainWindow::renameSongDirCheckedVideo(QUSongFile *song) {
	QString done("Song directory renamed from: \"%1\" to: \"%2\".");
	QString fail("Song directory: \"%1\" was NOT renamed.");
	
	QString oldName(song->songFileInfo().dir().dirName());
	QString newName("%1 - %2");
	
	if(song->hasVideo())
		newName.append(" [VIDEO]");
	
	if(song->edition().contains("[SC]", Qt::CaseInsensitive))
		newName.append(" [SC]");
	
	if(song->renameSongDir(newName.arg(song->artist()).arg(song->title()))) {
		addLogMsg(done.arg(oldName).arg(song->songFileInfo().dir().dirName()));
	} else {
		addLogMsg(fail.arg(oldName), 1);
	}
}

void QUMainWindow::renameSongTxt(QUSongFile *song) {
	QString done("Song text file renamed from: \"%1\" to: \"%2\".");
	QString fail("Song text file: \"%1\" was NOT renamed.");
	
	QString oldName(song->songFileInfo().fileName());
	
	if(song->renameSongTxt(song->artist() + " - " + song->title() + ".txt")) {
		addLogMsg(done.arg(oldName).arg(song->songFileInfo().fileName()));
	} else {
		addLogMsg(fail.arg(oldName), 1);
	}	
}

void QUMainWindow::renameSongMp3(QUSongFile *song) {
	QString done("Audio file renamed from: \"%1\" to: \"%2\".");
	QString fail("Audio file: \"%1\" was NOT renamed.");
	
	QString oldName(song->mp3());
	
	if(song->renameSongMp3(song->artist() + " - " + song->title() + "." + QFileInfo(song->mp3()).suffix().toLower())) {
		addLogMsg(done.arg(oldName).arg(song->mp3()));
	} else {
		addLogMsg(fail.arg(oldName), 1);
	}		
}

void QUMainWindow::renameSongCover(QUSongFile *song) {
	QString done("Cover file renamed from: \"%1\" to: \"%2\".");
	QString fail("Cover file: \"%1\" was NOT renamed.");

	QString oldName(song->cover());
	
	if(song->renameSongCover(song->artist() + " - " + song->title() + " [CO]." + QFileInfo(song->cover()).suffix().toLower())) {
		addLogMsg(done.arg(oldName).arg(song->cover()));
	} else {
		addLogMsg(fail.arg(oldName), 1);
	}	
}

void QUMainWindow::renameSongBackground(QUSongFile *song) {
	QString done("Background file renamed from: \"%1\" to: \"%2\".");
	QString fail("Background file: \"%1\" was NOT renamed.");
	
	QString oldName(song->background());
	
	if(song->renameSongBackground(song->artist() + " - " + song->title() + " [BG]." + QFileInfo(song->background()).suffix().toLower())) {
		addLogMsg(done.arg(oldName).arg(song->background()));
	} else {
		addLogMsg(fail.arg(oldName), 1);
	}	
}

void QUMainWindow::renameSongVideo(QUSongFile *song) {
	QString done("Video file renamed from: \"%1\" to: \"%2\".");
	QString fail("Video file: \"%1\" was NOT renamed.");
	
	QString oldName(song->video());
	
	if(song->renameSongVideo(song->artist() + " - " + song->title() + "." + QFileInfo(song->video()).suffix().toLower())) {
		addLogMsg(done.arg(oldName).arg(song->video()));
	} else {
		addLogMsg(fail.arg(oldName), 1);
	}	
}

void QUMainWindow::renameSongVideogap(QUSongFile *song) {
	QString done("Video file renamed from: \"%1\" to: \"%2\".");
	QString fail("Video file: \"%1\" was NOT renamed.");
	
	QString oldName(song->video());
	QString newName("%1 - %2 [VD#%3]." + QFileInfo(song->video()).suffix().toLower());
	
	if(song->renameSongVideo(newName.arg(song->artist()).arg(song->title()).arg(song->videogap()))) {
		addLogMsg(done.arg(oldName).arg(song->video()));
	} else {
		addLogMsg(fail.arg(oldName), 1);
	}		
}

void QUMainWindow::addLogMsg(const QString &msg, int type) {
	log->insertItem(0, QDateTime::currentDateTime().toString("[hh:mm:ss] ") + msg);
	
	switch(type) {
	case 1: log->item(0)->setIcon(QIcon(":/marks/error.png")); break;
	case 2: log->item(0)->setIcon(QIcon(":/marks/help.png")); break;
	default: log->item(0)->setIcon(QIcon(":/marks/information.png")); break;
	}
	
}

void QUMainWindow::aboutQt() {
	QMessageBox::aboutQt(this, "About Qt");
}

void QUMainWindow::aboutUman() {
	QMessageBox::about(this, "About", "<b>UltraStar Manager</b><br>Version 1.4<br><br>�2008 by Marcel Taeumel<br><br><i>Tested By</i><br>Michael Gr�newald");
}

void QUMainWindow::editTagOrder() {
	QUTagOrderDialog *dlg = new QUTagOrderDialog(this);
	
	dlg->exec();

	delete dlg;
	
	montyTalk();
}

void QUMainWindow::montyTalk() {
	if(!actionAllowMonty->isChecked())
		return;
	
	helpFrame->show();
	monty->talk(montyLbl, helpLbl);
}
