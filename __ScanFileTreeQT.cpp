/*
	Although you can't set a size of the branch directly you can manipulate it indirectly like this:
	style="box-sizing: border-box;">QTreeView::item { height: 100px; } // manipulates item but influences the height of the branch too
	QTreeView { qproperty-indentation: 100; } // offsets an item and so also widens the branch, note no unit (px)
*/


#include "__ScanFileTreeQT.h"

#define ATTR_NORMAL	0
#define ATTR_HIDDEN	1

#include <QPainter>
#include <QHeaderView>



ScanFileTreeQT::ScanFileTreeQT(QTreeWidget *TreeViewObj, bool sMode) : Tree(TreeViewObj), FastSearch(sMode)
{
	connect(Tree, SIGNAL(itemExpanded (QTreeWidgetItem*)), this, SLOT(itemExpanded (QTreeWidgetItem*)));
	connect(Tree, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(itemCollapsed(QTreeWidgetItem*)));

	init();
	ScanHardDrives();
}

ScanFileTreeQT::~ScanFileTreeQT()
{
	lastPathToFile();

	delete normalDirIcon;
	delete hiddenDirIcon;
	delete driveIcon;
	delete Tree;
}

// слот, принимающий сигнал о раскрытии ноды
void ScanFileTreeQT::itemExpanded(QTreeWidgetItem *item)
{
	//item->setTextColor(0, RGB(66, 132, 255));
	NodeExpand(item);
}

// слот, принимающий сигнал о закрытии ноды
void ScanFileTreeQT::itemCollapsed(QTreeWidgetItem *item)
{
	//QTreeWidgetItem *Parent_item = item->parent();
}

// настраиваем наш treeView
void ScanFileTreeQT::init()
{
	Tree->setColumnCount(1);
	Tree->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);
	Tree->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);	// можем выделять несколько папок с зажатым Ctrl
	Tree->setIconSize(QSize(32, 32));
	Tree->setFont(QFont("MS Sans Serif"));
	Tree->setAnimated(false);
	Tree->setHeaderHidden(true);
	Tree->setIndentation(40);

	// установим стили виджета QTreeWidget (наши большие кнопки '+'/'-')
	Tree->setStyleSheet(
		/*
		"QTreeWidget {"
			"background-color: yellow;"
		"}"*/

		// добавляем изображения, см. файл Resource Files/<name>.qrc
		"QTreeView::branch:has-children:!has-siblings:closed, QTreeView::branch:closed:has-children:has-siblings {\
			border-image: none;\
			image: url(:/qt_002_dirTree/images/treeBranch-closed.png);\
		}"

		"QTreeView::branch:open:has-children:!has-siblings, QTreeView::branch:open:has-children:has-siblings {\
			border-image: none;\
			image: url(:/qt_002_dirTree/images/treeBranch-open.png);\
		}"
	);

	Mask = "*.*";

	// настроим иконки
	normalDirIcon = new QIcon();
	hiddenDirIcon = new QIcon();
	driveIcon     = new QIcon();

	// иконка для дисков
	*driveIcon = Tree->style()->standardIcon(QStyle::SP_DriveHDIcon);

	// обычная иконка для папок в дереве
	*normalDirIcon = Tree->style()->standardIcon(QStyle::SP_DirIcon);

	// скрытая иконка для скрытых папок в дереве
	QPixmap normalPixmap((*normalDirIcon).pixmap((*normalDirIcon).actualSize(QSize(32, 32))));
	QPixmap disabledPixmap(QSize(32, 32));
	disabledPixmap.fill(Qt::transparent);
	QPainter painter(&disabledPixmap);

	painter.setBackgroundMode(Qt::TransparentMode);
	painter.setBackground(QBrush(Qt::transparent));
	painter.eraseRect(normalPixmap.rect());

	painter.setOpacity(0.5);
	painter.drawPixmap(0, 0, normalPixmap);
	painter.end();

	(*hiddenDirIcon).addPixmap(disabledPixmap, QIcon::Disabled, QIcon::On);
}

// находим все существующие жетские диски и строим первый уровень дерева файлов
void ScanFileTreeQT::ScanHardDrives()
{
	Tree->clear();

	WCHAR	Drives[255];
	QString	Drive;
	WCHAR*	DrivePointer;

	// получить список всех доступных дисков; если их больше чем 256/4, то стоп
	if (GetLogicalDriveStrings(256, Drives) <= 256) {

		DrivePointer = Drives;
		WCHAR fileSystem[80], volumeName[80];
		unsigned long p1 = 80;

		while (*DrivePointer != 0) {
			Drive = QString::fromStdWString(DrivePointer);
			int driveType = GetDriveType(DrivePointer);

			GetVolumeInformation(Drive.toStdWString().data(), volumeName, 80, &p1, &p1, &p1, fileSystem, p1);

			// для каждого постоянного или съемного диска построим дерево
			// сидиромы и флоппи диски не интересуют
			// driveType == DRIVE_REMOVABLE не используется, т.к. опрашивает флоппи и выдает ошибку
			if (driveType == DRIVE_FIXED)
				BuildTree(NULL, Drive, QString::fromStdWString(volumeName));

			// смещаемся на 4 позиции по массиву Drives
			DrivePointer += 4;
		}
	}
}

void ScanFileTreeQT::BuildTree(QTreeWidgetItem *Node, const QString Path, const QString volumeName = "")
{
	// включение корневых каталогов в дерево
	if (Node == NULL) {

		QTreeWidgetItem *upperLevelNode = new QTreeWidgetItem(Tree);

		upperLevelNode->setText(0, volumeName + " (" + Path.section('\\', 0, 0) + ")");
		upperLevelNode->setExpanded(false);
		upperLevelNode->setSizeHint(0, QSize(0, 30));
		upperLevelNode->setIcon(0, *driveIcon);

		Tree->addTopLevelItem(upperLevelNode);

		Node = upperLevelNode;
	}

	QTreeWidgetItem *newNode;

	// маска поиска файлов и директорий
	QString Mask = Path + "\\" + "*.*", nodePath, path, mask;

	// цикл добавления новых узлов в дерево
	for (CScanFolder SF(Mask, 0); SF; SF++)
	{
		nodePath = QString::fromWCharArray(SF);
		int attr;

		if ((nodePath != ".") && (nodePath != ".."))
		{
			// добавление директорий
			if (pathIsDirectory(Path + nodePath, attr))
			{
				QTreeWidgetItem *newNode = new QTreeWidgetItem();

				newNode->setText(0, nodePath);

				if( attr == ATTR_HIDDEN )
					newNode->setIcon(0, *hiddenDirIcon);
				else
					newNode->setIcon(0, *normalDirIcon);

				newNode->setExpanded(false);
				newNode->setSizeHint(0, QSize(0, 30));

				Node->addChild(newNode);

				// создаем хитрую директорию и добавляем ее в нашу новую директорию, чтобы у нее появился значок '+'
				QTreeWidgetItem *secretNode = new QTreeWidgetItem();
				secretNode->setText(0, "...");

				if (FastSearch == true)
				{
					// ускоренный обзор:
					// поиск не проводится ниже текущего уровня, у каждой папки в дереве есть значок '+', даже если в ней нет подпапок
					newNode->addChild(secretNode);
				}
				else
				{
					// чуть более медленный обзор:
					// ищет на уровень глубже и если находит хотя бы одну вложенную папку, то создает хитрую директорию и прерывает поиск.
					// если в папке нету подпапок, то у нее не будет значка '+'
					path = Path + "\\" + nodePath;
					mask = path + "\\" + "*.*";
					int attr;

					for (CScanFolder SF(mask, 0); SF; SF++)
					{
						QString innerPath = QString::fromWCharArray(SF);

						if ((innerPath != ".") && (innerPath != ".."))
						{
							if (pathIsDirectory(path + "\\" + innerPath, attr))
							{
								// создание хитрой несуществующей директории (если в папке есть хотя бы одна подпапка)
								// для создания иллюзии того, что папка не пуста. При раскрытии узла эта директория
								// удаляется и происходит фактический просмотр папок
								newNode->addChild(secretNode);
								break;
							}
						}
					}
				}
			}
			else
			{
				// добавление файлов в дерево. Здесь не нужно, т.к. файлы обрабатываются в функции findFiles()
				// newNode = Tree->Items->AddChild(Node, (AnsiString(SF)));
				// newNode->ImageIndex = 1;
				// newNode->SelectedIndex = newNode->ImageIndex;
			}
		}
	}

	Node->sortChildren(0, Qt::SortOrder::AscendingOrder);
}

// проверяем, является ли файл директорией
bool ScanFileTreeQT::pathIsDirectory(QString path, int &attr)
{
	LPCWSTR Path  = (const wchar_t*)path.utf16();
	DWORD dwAttrs = GetFileAttributes(Path);
	attr          = ATTR_NORMAL;

	// файл только для чтения
	if (dwAttrs & FILE_ATTRIBUTE_READONLY) {
		dwAttrs &= ~FILE_ATTRIBUTE_READONLY;
	}

	// файл скрытый
	if (dwAttrs & FILE_ATTRIBUTE_HIDDEN) {
		dwAttrs &= ~FILE_ATTRIBUTE_HIDDEN;
		attr = ATTR_HIDDEN;
	}

	// файл системный
	if (dwAttrs & FILE_ATTRIBUTE_SYSTEM) {
		dwAttrs &= ~FILE_ATTRIBUTE_SYSTEM;
		attr = ATTR_HIDDEN;
	}

	// файл является директорией
	if (dwAttrs & FILE_ATTRIBUTE_DIRECTORY ) {
		if(path.contains("pagefile.sys")) {
			attr = ATTR_HIDDEN;
		}
		else  {
			dwAttrs &= ~FILE_ATTRIBUTE_DIRECTORY;
			return true;
		}
	}

	return false;
}

// вызываем этот метод при раскрытии ноды
void ScanFileTreeQT::NodeExpand(QTreeWidgetItem *Node)
{
	// выделяем ноду
	Tree->setCurrentItem(Node, 0);

	// раскрываем ранее нераскрытую директорию, в которой хитро спрятан потомок с именем "..."
	if ((Node->childCount() == 1) && (Node->child(0)->text(0) == "...")) {
		Node->removeChild(Node->child(0));
		BuildTree(Node, getPath(Node));
	}
}

// получим полный путь к заданной директории/ноде
QString ScanFileTreeQT::getPath(QTreeWidgetItem *Node)
{
	QTreeWidgetItem *currentNode = Node;
	QString Path = "", root;

	// собираем полный путь к директории снизу вверх
	while (currentNode->parent() != NULL) {
		Path.prepend(currentNode->text(0) + "\\");
		currentNode = currentNode->parent();
	}

	root = currentNode->text(0);
	root.chop(root.length() - root.indexOf(':') - 1);
	Path.prepend(root.right(2) + "\\");

	return Path;
}

QString ScanFileTreeQT::getCurrNodeName()
{
	QTreeWidgetItem *currentNode = Tree->selectedItems()[0];
	return getPath(currentNode);
}

// заполняем предоставленный список файлами, находящимися в выбранных директориях
void ScanFileTreeQT::getFileList(QStringList& list, qMapSS& filesMap, qMapSC& ExtensionsMap)
{
	list.clear();
	filesMap.clear();

	QList<QTreeWidgetItem*> Nodes = Tree->selectedItems();

	// проходим по всем нодам, для каждой ноды получаем список файлов и добавляем его в наш список
	for(int i = 0; i < Nodes.length(); i++)
		findFiles(getPath(Nodes[i]), list, filesMap, ExtensionsMap);
}

// непосредственно находим файлы в указанной директории
void ScanFileTreeQT::findFiles(QString Path, QStringList& list, qMapSS& filesMap, qMapSC& ExtensionsMap)
{
	QString nodePath;
	bool includeSub = false;
	bool includeDir = false;

	list.push_back( Path + nodePath + "?");

    for(CScanFolder SF(Path + "*.*", 0); SF; SF++) {

		nodePath = QString::fromWCharArray(SF);
		int attr;

		// включаем в список файлы в подпапках, если выставлена соответствующая настройка
		if( pathIsDirectory(Path + nodePath, attr) ) {

			if( includeSub && nodePath != "." && nodePath != ".." )
				findFiles(Path + nodePath, list, filesMap, ExtensionsMap);

			if( includeDir ) {
				list.push_back(Path + nodePath);
			}
		}
        else {

			// включаем в список файлы в текущей папке
			//if( allowByMask(AnsiString(SF)) )
            {
				//list.push_back( Path[Path.length()-1] == '\\' ? Path + nodePath : Path + '\\' + nodePath);	// полный путь
				// в список кладем только имя файла
				list.push_back( nodePath );
				// в map кладем полный путь
				QString fullPath = Path[Path.length()-1] == '\\' ? Path + nodePath : Path + '\\' + nodePath;
				filesMap.insert(fullPath, fullPath);

				QString ext = nodePath.right(nodePath.length() - nodePath.lastIndexOf('.'));

				ExtensionsMap.insert(ext, '.');
			}
		}
	}

	return;
}

void ScanFileTreeQT::lastPathToFile()
{
	QList<QTreeWidgetItem*> currentNodeList = Tree->selectedItems();

	if( currentNodeList.length() > 0 ) {
		QString path = getPath(currentNodeList[0]);
		std::ofstream outFile("_path");
		outFile << path.toStdString().data();
		outFile.close();
	}
}

QString ScanFileTreeQT::lastPathFromFile()
{
	QString path = "";
    char ch[MAX_PATH];
    std::ifstream inFile("_path");

    while( inFile >> ch ) {
		path += ch;
        path += " ";	// wtf? но без этой строчки из пути исчезают все пробелы...
	}

    //path = path.TrimRight();
    inFile.close();

    return path;
}

void ScanFileTreeQT::setLastDirectory()
{
	setDirectory(lastPathFromFile());
}

// открываем в дереве последнюю папку из прошлой сессии
void ScanFileTreeQT::setDirectory(QString path)
{
	path = path.toLower();
	QStringList dirs = path.split("\\");
	QTreeWidgetItem* currentNode = Tree->invisibleRootItem();

	// проходим по всем участкам нашего пути к папке
	for(int i = 0; i < dirs.length(); i++) {
	
		path = dirs[i];
		bool found = false;

		// проходим по всем потомкам текущей ноды в дереве
		for(int j = 0; j < currentNode->childCount(); j++) {

			QString nodeName = currentNode->child(j)->text(0).toLower();

			// если текст потомка равен текущей части пути (или содержит букву диска + ':'), проваливаемся на уровень глубже
			if( path == nodeName || ( i == 0 && nodeName.contains(path) ) ) {
				found = true;
				currentNode = currentNode->child(j);
				break;
			}
		}

		if( found )
			currentNode->setExpanded(true);
		else
			break;
	}

	return;
}

// перечитаем все отмеченные директории (вызываем этот метод по внешнему событию, например, по кнопке)
// чтобы перечитать файлы в этих директориях, нужно в том же самом событии после этого метода вызвать getFileList(filesList);
void ScanFileTreeQT::reReadSelectedDir()
{
	QList<QTreeWidgetItem*> selected = Tree->selectedItems();

	for(int i = 0; i < selected.length(); i++) {

		QTreeWidgetItem *item = selected[i];
		QString path = getPath(item);

		bool isExpanded = item->isExpanded();

		for(int j = item->childCount()-1; j >= 0; j--)
			item->removeChild(item->child(j));

		BuildTree(item, path);

		if( isExpanded )
			item->setExpanded(true);

		// посылаем сигнал о том, что изменилось выделение в дереве, чтобы перечитать файлы в папке
		Tree->itemSelectionChanged();
	}
}