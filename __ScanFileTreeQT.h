#ifndef ___SCANFILETREEQT_H
#define ___SCANFILETREEQT_H

#include <QObject>

#include <fstream>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>

#include "__ScanFolder.h"

typedef QMap<QString, QString>	qMapSS;
typedef QMap<QString, char>		qMapSC;



class ScanFileTreeQT : public QObject {
	Q_OBJECT

 public:
	ScanFileTreeQT(QTreeWidget *TreeViewObj, bool sMode = false);
	~ScanFileTreeQT();

 public:
	// первичное раскрытие ранее нераскрытой ноды; используется по сигналу itemExpanded(QTreeWidgetItem*)
	void NodeExpand(QTreeWidgetItem*);

	QString getCurrNodeName();

	// получаем список файлов (QStringList), находящихся под выделенной директорией/директориями
	void getFileList(QStringList&, qMapSS&, qMapSC&);

	// перестроить дерево для отмеченной папки / папок
	void reReadSelectedDir();

	// sort function; use it with the 'TreeView1Compare()' event
	//int NodeCompare(TTreeNode*, TTreeNode*);

	// adds all the found files to suplied vector; use it with the 'TreeView1Change()' event
	//void filesToVector(TTreeNode*, vector<AnsiString>&);

	// установка маски для поиска файлов по ней
	//void setMask(const char*);

	// flips IncludeSub flag from false to true and vice versa
	//void SubDirFlip();

	// открыть в дереве последнюю использованную папку по заданному пути (или максимально к ней близкую, если такой папки уже не существует)
	// не можем вызывать этот метод из конструктора, т.к. по сигналу вызывается слот, который вызывает метод объекта класса ScanFileTreeQT, который еще не готов, т.к. конструктор не отработал до конца
	void setLastDirectory();

 private:
	void init();
	void ScanHardDrives();
	void BuildTree(QTreeWidgetItem*, const QString, const QString);
	bool pathIsDirectory(QString, int&);
	void findFiles(QString, QStringList&, qMapSS&, qMapSC&);

	// Получить полный путь к каталогу, адресуемому нодой
	QString getPath(QTreeWidgetItem*);

	// Сохранить и прочитать из файла последний использованный путь к папке дерева
	void lastPathToFile();
	QString lastPathFromFile();

	// открыть в дереве папку по заданному пути (или максимально к ней близкую, если такой папки уже не существует)
	void setDirectory(QString);

 public slots:
	void itemExpanded(QTreeWidgetItem*);
	void itemCollapsed(QTreeWidgetItem*);

 private:
	QTreeWidget* Tree;
	bool		 FastSearch;
	QString		 Mask;
	QIcon*		 normalDirIcon;
	QIcon*		 hiddenDirIcon;
	QIcon*		 driveIcon;
};

#endif // ___SCANFILETREEQT_H
