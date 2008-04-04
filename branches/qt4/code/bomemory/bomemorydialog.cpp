/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include "bomemorydialog.h"
#include "bomemorydialog.moc"

#include <config.h>
#include "memnode.h"
#include "memorymanager.h"
#include <bodebug.h>

#include <klocale.h>
#include <klistview.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qptrdict.h>
#include <qptrlist.h>
#include <qdict.h>

class MyMemNode {
public:
	MyMemNode()
	{
		mSize = 0;
		mPointer = 0;
		mLine = -1;
		mFile = 0;
		mFunction = 0;
		mIsMalloc = false;
	}
	~MyMemNode()
	{
		delete[] mFile;
		delete[] mFunction;
	}
	static bool checkNode(MemNode* node)
	{
		// AB: sometimes there are invalid pointers in our version of
		// new() in the strings. i have no idea why and where they come
		// from - every // function (including e.g. strlen()) is
		// crashing on these pointers.
		// they don't occur _every_ time, but very often.
		if (((unsigned int)node->mFile) < 0xFF) {
			boError() << k_funcinfo << "invalid mFile string pointer:" << (void*)node->mFile << " won't use this node." << endl;
			return false;
		}
		if (((unsigned int)node->mFunction) < 0xFF) {
			boError() << k_funcinfo << "invalid mFunction string pointer:" << (void*)node->mFile << " won't use this node." << endl;
			return false;
		}
		if (node->mLine < -1) {
			boError() << k_funcinfo << "invalid mLine number:" << node->mLine << " won't use this node." << endl;
		}
		return true;
	}
	bool loadNode(MemNode* node)
	{
		if (!checkNode(node)) {
			boError() << k_funcinfo << "something weird happened" << endl;
			mFile = new char[1];
			mFile[0] = '\0';
			mFunction = new char[1];
			mFunction[0] = '\0';
			mIsMalloc = false;
			return false;
		}
		mSize = node->mSize;
		mPointer = node->mPointer;
		mLine = node->mLine;

		int len;
		len = strlen(node->mFile) + 1;
		mFile = new char[len];
		strncpy(mFile, node->mFile, len);
		mFile[len - 1] = '\0';
		len = strlen(node->mFunction) + 1;
		mFunction = new char[len];
		strncpy(mFunction, node->mFunction, len);
		mFunction[len - 1] = '\0';
		mIsMalloc = node->mIsMalloc;
		return true;
	}
	size_t mSize;
	void* mPointer;
	int mLine;
	char* mFile;
	char* mFunction;
	bool mIsMalloc;
};

class BoMemoryDialogPrivate
{
public:
	BoMemoryDialogPrivate()
	{
		mNodeCount = 0;
		mMemory = 0;
		mList = 0;
	}
	QLabel* mNodeCount;
	QLabel* mMemory;
	KListView* mList;
};

BoMemoryDialog::BoMemoryDialog(QWidget* parent, bool modal)
		: KDialogBase(Plain, i18n("Boson Memory Usage"), Ok,
		Ok, parent, "bomemorydialog", modal, true)
{
 d = new BoMemoryDialogPrivate;
 QVBoxLayout* layout = new QVBoxLayout(plainPage());
 d->mNodeCount = new QLabel(plainPage());
 layout->addWidget(d->mNodeCount);
 d->mMemory = new QLabel(plainPage());
 layout->addWidget(d->mMemory);
 d->mList = new KListView(plainPage());
 d->mList->setAllColumnsShowFocus(true);
 d->mList->setRootIsDecorated(true);
 layout->addWidget(d->mList);
 d->mList->addColumn(i18n("File"));
 d->mList->addColumn(i18n("Function"), 100);
 d->mList->addColumn(i18n("Line"));
 d->mList->addColumn(i18n("Size (Byte)"));
 d->mList->addColumn(i18n("Size (KB)"));
 d->mList->addColumn(i18n("Size (MB)"));
 d->mList->addColumn(i18n("malloc calls"));
 d->mList->addColumn(i18n("new calls"));
}

BoMemoryDialog::~BoMemoryDialog()
{
 boDebug() << k_funcinfo << endl;
 delete d;
}


void BoMemoryDialog::slotUpdate()
{
 boDebug() << k_funcinfo << endl;
 d->mList->clear();
 MemoryManager::createManager();
 if (!MemoryManager::manager()) {
	boError() << k_funcinfo << "NULL memory manager. probably permanently disabled. cannot show any data" << endl;
	return;
 }
 MemoryManager::manager()->disable(); // do not manage the ptrdict
 unsigned int nodeCount = MemoryManager::manager()->allNodes().count();
 unsigned int totalMemory = MemoryManager::manager()->memoryInUse();
 QPtrDict<MemNode>* _allNodes = new QPtrDict<MemNode>(MemoryManager::manager()->allNodes());
 MemoryManager::manager()->enable();

 // make sure all nodes are valid and they _stay_ valid (we can't know if Qt
 // or something else will delete anything while we are still working on these
 // data)
 QPtrDict<MyMemNode> allNodes(_allNodes->size());
 allNodes.setAutoDelete(true);
 { // start a new scope to make sure _allNodesIt gets destructed
	QPtrDictIterator<MemNode> _allNodesIt(*_allNodes);
	while (_allNodesIt.current()) {
		MemNode* n = _allNodesIt.current();
		void* key = _allNodesIt.currentKey();
		++_allNodesIt;

		if (!MyMemNode::checkNode(n)) {
			continue;
		}

		MyMemNode* node = new MyMemNode();
		if (!node->loadNode(n)) {
			delete node;
		}
		allNodes.insert(key, node);
	}
 }
 MemoryManager::manager()->disable();
 _allNodes->clear();
 delete _allNodes;
 _allNodes = 0;
 MemoryManager::manager()->enable();

 d->mNodeCount->setText(i18n("Total nodes: %1").arg(nodeCount));
 d->mMemory->setText(i18n("Memory in use (Bytes/KB/MB): %1 / %2 / %3").arg(totalMemory).
		arg( ((double)totalMemory) / (1024.0)).
		arg( ((double)totalMemory) / (1024.0 * 1024.0) ));

 boDebug() << k_funcinfo << "processing " << allNodes.count() << " nodes" << endl;

 unsigned long int mmemory = 0;
 unsigned long int nmemory = 0;
 // AB: a ptr dict is pretty useless to us, we need something else
 QPtrDictIterator<MyMemNode> dictIt(allNodes);
 QDict<QPtrList<MyMemNode> > file2NodeList;
 file2NodeList.setAutoDelete(true);
 while (dictIt.current()) {
	MyMemNode* node = dictIt.current();
	++dictIt;

	if (node->mIsMalloc) {
		mmemory += node->mSize;
	} else {
		nmemory += node->mSize;
	}

	if (!node->mFile) {
		boError() <<k_funcinfo << "NULL file string" <<  endl;
		continue;
	}

	// AB: sometimes there are invalid pointers in our version of new() in
	// the strings. i have no idea why and where they come from - every
	// function (including e.g. strlen()) is crashing on these pointers.
	// they don't occur _every_ time, but very often.
	if (((unsigned int)node->mFile) < 0xFF) {
		boError() << k_funcinfo << "invalid mFile string pointer:" << (void*)node->mFile << " won't use this node." << endl;
		continue;
	}
	if (((unsigned int)node->mFunction) < 0xFF) {
		boError() << k_funcinfo << "invalid mFunction string pointer:" << (void*)node->mFile << " won't use this node." << endl;
		continue;
	}
	QString file = QString(node->mFile);
	QPtrList<MyMemNode>* list = file2NodeList[file];
	if (!list) {
		list = new QPtrList<MyMemNode>;
		file2NodeList.insert(file, list);
	}
	list->append(node);
 }
 boWarning() << k_funcinfo << "malloc'ed memory: " << mmemory << endl;
 boWarning() << k_funcinfo << "new'ed memory: " << nmemory << endl;

 unsigned long int size = 0;
 QDictIterator<QPtrList<MyMemNode> > nodeIt(file2NodeList);
 while (nodeIt.current()) {
	QString fileName = nodeIt.currentKey();
	QPtrList<MyMemNode>* list = nodeIt.current();
	++nodeIt;

	QListViewItem* file = createFileItem(fileName);

//	boDebug() << k_funcinfo << "creating items for file " << fileName << endl;
	size += createFileList(file, list);
 }
 boDebug() << k_funcinfo << "total size: " << size << endl;
 allNodes.clear();
}

QListViewItem* BoMemoryDialog::createFileItem(const QString& file) const
{
 QListViewItem* item = new QListViewItem(d->mList, file);
 item->setOpen(false);
 return item;
}

QListViewItem* BoMemoryDialog::createFunctionItem(QListViewItem* parent, const QString& function) const
{
 QListViewItem* item = new QListViewItem(parent, "", function);
 item->setOpen(false);
 return item;
}

QListViewItem* BoMemoryDialog::createMemoryItem(QListViewItem* parent, const MyMemNode* node) const
{
 if (!node) {
	return 0;
 }
 QString line = QString::number(node->mLine);
 QListViewItem* item = new QListViewItem(parent, node->mFile, node->mFunction, line);
 setSize(item, node->mSize);
 unsigned int mallocCalls = node->mIsMalloc? 1 : 0;
 unsigned int newCalls = node->mIsMalloc? 0 : 1;
 QString is = (node->mIsMalloc) ? QString("malloc") : QString("new");
 item->setText(6, QString::number(mallocCalls));
 item->setText(7, QString::number(newCalls));
 return item;
}

void BoMemoryDialog::setSize(QListViewItem* item, unsigned long int size_) const
{
 QString size;
 QString sizeK;
 QString sizeM;
 size.sprintf("%07d", (int)size_);
 sizeK.sprintf("%07f", ((double)size_) / (1024.0));
 sizeM.sprintf("%07f", ((double)size_) / (1024.0 * 1024.0));

 item->setText(3, size);
 item->setText(4, sizeK);
 item->setText(5, sizeM);
}


unsigned long int BoMemoryDialog::createFileList(QListViewItem* file, const QPtrList<MyMemNode>* list_)
{
 if (!list_ || !file) {
	return 0;
 }
 QPtrList<MyMemNode> list = *list_;
// boDebug() << k_funcinfo << list.count() << endl;
 list.sort();
 unsigned long int size = 0;

 while (!list.isEmpty()) {
	MyMemNode* node = list.first();
	list.removeFirst();
	const char* function = node->mFunction;
//	boDebug() << k_funcinfo << "left: " << list.count() << endl;

	QPtrList<MyMemNode> sameFunction;
	QPtrList<MyMemNode> otherFunction;
	sameFunction.append(node);
	QPtrListIterator<MyMemNode> listIt(list);
	while (listIt.current()) {
		MyMemNode* n = listIt.current();
		++listIt;

		if (strncmp(function, n->mFunction, 256) == 0) {
			sameFunction.append(n);
		} else {
			otherFunction.append(n);
		}
	}
	list = otherFunction;

	QListViewItem* item = createFunctionItem(file, QString(function));
	size += createFunctionList(item, &sameFunction);
 }
// boDebug() << k_funcinfo << "done" << endl;
 setSize(file, size);
 return size;
}

unsigned long int BoMemoryDialog::createFunctionList(QListViewItem* function, const QPtrList<MyMemNode>* list)
{
 if (!function || !list) {
	return 0;
 }
 unsigned long int size = 0;
 unsigned int mallocCalls = 0;
 unsigned int newCalls = 0;

 QPtrListIterator<MyMemNode> it(*list);
 while (it.current()) {
	MyMemNode* node = it.current();
	++it;

#if 0
	createMemoryItem(function, node);
#endif
	size += node->mSize;
	if (node->mIsMalloc) {
		mallocCalls++;
	} else {
		newCalls++;
	}
 }
 setSize(function, size);
 function->setText(6, QString::number(mallocCalls));
 function->setText(7, QString::number(newCalls));
 return size;
}

