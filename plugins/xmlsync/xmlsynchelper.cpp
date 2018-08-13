#include "xmlsynchelper.h"
#include <QDateTime>
#include <QDebug>
#include <QSaveFile>

const QString XmlSyncHelper::ModeXml = QStringLiteral("xml");

XmlSyncHelper::XmlSyncHelper(QObject *parent) :
	SyncHelper(parent)
{}

bool XmlSyncHelper::pathIsPattern(const QString &mode) const
{
	Q_UNUSED(mode)
	return true;
}

bool XmlSyncHelper::canSyncDirs(const QString &mode) const
{
	Q_UNUSED(mode)
	return false;
}

SyncHelper::ExtrasHint XmlSyncHelper::extrasHint() const
{
	return {
		true,
		tr("&Keys"),
		tr("<p>Enter the paths of the XML-Elements you want to synchronize. Elements are seperated by a '/' "
		   "and there are some special characters that match special things. The possible variants are:"
		   "<ul>"
		   "	<li>\"some/node\": A path of elements. This will synchronize the element \"node\" within the"
		   "		element \"some\" inside the root element. All attributes, child-elements and text are synchronized</li>"
		   "	<li>\"node/~\": This will synchronize the text within the \"node\" element, and nothing else</li>"
		   "	<li>\"node/=\": This will synchronize all attributes of \"node\" but not the contents</li>"
		   "	<li>\"node/=key\": This will synchronize the attribute \"key\" of \"node\" but not the contents</li>"
		   "	<li>\"node/#\": This will synchronize all attributes and content text of \"node\"</li>"
		   "	<li>\"node/\": This will synchronize all child elements in \"node\", but no attributes</li>"
		   "</ul></p>"
		   "<p>You never have to specify the root element, as a XML-Document can only have a single root.</p>")
	};
}

SyncTask *XmlSyncHelper::createSyncTask(QString mode, const QDir &syncDir, QString path, QStringList extras, bool isFirstUse, QObject *parent)
{
	return new XmlSyncTask {
		this,
		std::move(mode),
		syncDir,
		std::move(path),
		std::move(extras),
		isFirstUse,
		parent
	};
}

SyncTask *XmlSyncHelper::createUndoSyncTask(QString mode, const QDir &syncDir, QString path, QObject *parent)
{
	return new XmlSyncTask {
		this,
		std::move(mode),
		syncDir,
		std::move(path),
		parent
	};
}



QMutex XmlSyncTask::_XmlMutex;

XmlSyncTask::XmlSyncTask(const XmlSyncHelper *helper, QString &&mode, const QDir &syncDir, QString &&path, QStringList &&extras, bool isFirstUse, QObject *parent) :
	SyncTask{helper, std::move(mode), syncDir, std::move(path), std::move(extras), isFirstUse, parent}
{}

XmlSyncTask::XmlSyncTask(const XmlSyncHelper *helper, QString &&mode, const QDir &syncDir, QString &&path, QObject *parent) :
	SyncTask{helper, std::move(mode), syncDir, std::move(path), parent}
{}

void XmlSyncTask::performSync()
{
	auto srcInfo = srcPath();
	auto syncInfo = syncPath();

	// step 1: load the two xml models
	auto srcDoc = loadDocument(srcInfo);
	auto syncDoc = loadDocument(syncInfo);
	if(srcDoc.documentElement().isNull() &&
	   syncDoc.documentElement().isNull())
		return;
	else if(srcDoc.documentElement().isNull())
		srcDoc = createDoc(syncDoc);
	else if(syncDoc.documentElement().isNull())
		syncDoc = createDoc(srcDoc);

	bool srcNeedsUpdate = false;
	bool syncNeedsUpdate = false;
	auto srcIsNewer = isFirstUse ? false : srcInfo.lastModified() > syncInfo.lastModified();

	// step 2: use extras to scan over both docs and transfer changes
	for(const auto& tree : extras) {
		auto chain = tree.split(QLatin1Char('/'));
		QDomElement srcElement = srcDoc.documentElement();
		QDomElement syncElement = syncDoc.documentElement();
		auto srcExists = true;
		auto syncExists = true;

		auto includeAttribs = true;
		auto lastElemNeeded = true;
		for(const auto& link : chain) {
			// handle text content
			if(link == QStringLiteral("~")) {
				updateText(srcElement, syncElement,
						   srcExists, syncExists,
						   srcIsNewer,
						   srcNeedsUpdate, syncNeedsUpdate,
						   tree);
				lastElemNeeded = false;
				break; //chain must end here
			// handle attribute content (all attributes)
			} else if(link == QStringLiteral("=")) {
				updateAttributes(srcElement, syncElement,
								 srcIsNewer,
								 srcNeedsUpdate, syncNeedsUpdate,
								 tree);
				lastElemNeeded = false;
				break; //chain must end here
			} else if(link.startsWith(QStringLiteral("="))) {
				auto name = link.mid(1);
				auto srcAttr = srcElement.attributeNode(name);
				auto syncAttr = syncElement.attributeNode(name);
				updateAttribute(srcElement, syncElement,
								srcAttr, syncAttr,
								srcIsNewer,
								srcNeedsUpdate, syncNeedsUpdate,
								tree);
				lastElemNeeded = false;
				break; //chain must end here
			// handle text content and all attributes
			} else if(link == QStringLiteral("#")) {
				updateText(srcElement, syncElement,
						   srcExists, syncExists,
						   srcIsNewer,
						   srcNeedsUpdate, syncNeedsUpdate,
						   tree);
				updateAttributes(srcElement, syncElement,
								 srcIsNewer,
								 srcNeedsUpdate, syncNeedsUpdate,
								 tree);
				lastElemNeeded = false;
				break; //chain must end here
			// handle child chain without attribs
			} else if(link == QString()) {
				includeAttribs = false;
				break; //chain must end here
			} else {
				srcElement = cd(srcElement, link, srcExists);
				syncElement = cd(syncElement, link, syncExists);
			}
		}

		// handle element sync
		if(lastElemNeeded) {
			// cant check for changed, so always assume changed
			if(srcExists && syncExists) {
				if(srcIsNewer) {
					auto syncParent = syncElement.parentNode();
					auto newChild = srcElement.cloneNode(true);
					if(!includeAttribs)
						removeAttribs(newChild.toElement());
					syncParent.replaceChild(newChild, syncElement);
					syncNeedsUpdate = true;
					info(tree) << "Updated nodes in sync from src";
				} else {
					auto srcParent = srcElement.parentNode();
					auto newChild = syncElement.cloneNode(true);
					if(!includeAttribs)
						removeAttribs(newChild.toElement());
					srcParent.replaceChild(newChild, srcElement);
					srcNeedsUpdate = true;
					info(tree) << "Updated nodes in src from sync";
				}
			} else if(srcExists) {
				auto syncParent = syncElement.parentNode();
				auto newChild = srcElement.cloneNode(true);
				if(!includeAttribs)
					removeAttribs(newChild.toElement());
				syncParent.replaceChild(newChild, syncElement);
				syncNeedsUpdate = true;
				info(tree) << "Added new nodes to sync from src";
			} else if(syncExists) {
				auto srcParent = srcElement.parentNode();
				auto newChild = syncElement.cloneNode(true);
				if(!includeAttribs)
					removeAttribs(newChild.toElement());
				srcParent.replaceChild(newChild, srcElement);
				srcNeedsUpdate = true;
				info(tree) << "Added new nodes to src from sync";
			}
		}
	}

	// step 3: write the files
	if(srcNeedsUpdate)
		saveDocument(srcInfo, srcDoc);
	if(syncNeedsUpdate)
		saveDocument(syncInfo, syncDoc);
}

void XmlSyncTask::updateText(const QDomElement &srcElement, const QDomElement &syncElement, bool srcExists, bool syncExists, bool srcIsNewer, bool &srcNeedsUpdate, bool &syncNeedsUpdate, const QString &key)
{
	// both have the text
	if(srcExists && syncExists) {
		auto srcText = getText(srcElement);
		auto syncText = getText(syncElement);
		// text is different
		if(srcText != syncText) {
			if(srcIsNewer) {
				writeText(syncElement, srcText);
				syncNeedsUpdate = true;
				info(key) << "Updated text in sync from src";
			} else {
				writeText(srcElement, syncText);
				srcNeedsUpdate = true;
				info(key) << "Updated text in src from sync";
			}
		} else
			debug(key) << "Skipping unchanged text";
	// only src has text
	} else if(srcExists) {
		writeText(syncElement, getText(srcElement));
		syncNeedsUpdate = true;
		info(key) << "Added new text in sync from src";
	// only sync has text
	} else if(syncExists) {
		writeText(srcElement, getText(syncElement));
		srcNeedsUpdate = true;
		info(key) << "Added new text in src from sync";
	}
}

void XmlSyncTask::updateAttributes(const QDomElement &srcElement, const QDomElement &syncElement, bool srcIsNewer, bool &srcNeedsUpdate, bool &syncNeedsUpdate, const QString &key)
{
	auto srcAttribs = srcElement.attributes();
	auto syncAttribs = syncElement.attributes();

	//from src to sync (and both)
	QSet<QString> skipNames;
	for(auto i = 0; i < srcAttribs.size(); i++) {
		auto srcAttr = srcAttribs.item(i).toAttr();
		auto name = srcAttr.name();
		auto syncAttr = syncAttribs.namedItem(name).toAttr();
		updateAttribute(srcElement, syncElement,
						srcAttr, syncAttr,
						srcIsNewer,
						srcNeedsUpdate, syncNeedsUpdate,
						key + name);
		skipNames.insert(name);
	}

	// from sync to src
	for(auto i = 0; i < syncAttribs.size(); i++) {
		auto syncAttr = syncAttribs.item(i).toAttr();
		auto name = syncAttr.name();
		if(skipNames.contains(name))
			continue;
		auto srcAttr = srcAttribs.namedItem(name).toAttr();
		updateAttribute(srcElement, syncElement,
						srcAttr, syncAttr,
						srcIsNewer,
						srcNeedsUpdate, syncNeedsUpdate,
						key + name);
	}
}

void XmlSyncTask::updateAttribute(QDomElement srcElement, QDomElement syncElement, QDomAttr srcAttr, QDomAttr syncAttr, bool srcIsNewer, bool &srcNeedsUpdate, bool &syncNeedsUpdate, const QString &key)
{
	// both have attrib
	if(!srcAttr.isNull() && !syncAttr.isNull()) {
		auto srcValue = srcAttr.value();
		auto syncValue = syncAttr.value();
		if(srcValue != syncValue) {
			if(srcIsNewer) {
				syncAttr.setValue(srcValue);
				syncNeedsUpdate = true;
				info(key) << "Updated attribute in sync from src";
			} else {
				srcAttr.setValue(syncValue);
				srcNeedsUpdate = true;
				info(key) << "Updated attribute in src from sync";
			}
		} else
			debug(key) << "Skipping unchanged attribute";
	// only src has attrib
	} else if(!srcAttr.isNull()) {
		syncAttr = syncElement.ownerDocument().createAttribute(srcAttr.name());
		syncAttr.setValue(srcAttr.value());
		syncElement.setAttributeNode(syncAttr);
		syncNeedsUpdate = true;
		info(key) << "Added new attribute in sync from src";
	// only sync has attrib
	} else if(!syncAttr.isNull()) {
		srcAttr = srcElement.ownerDocument().createAttribute(syncAttr.name());
		srcAttr.setValue(syncAttr.value());
		srcElement.setAttributeNode(srcAttr);
		srcNeedsUpdate = true;
		info(key) << "Added new attribute in src from sync";
	}
}

QDomDocument XmlSyncTask::loadDocument(const QFileInfo &file) const
{
	QFile readFile{file.absoluteFilePath()};
	if(!readFile.exists())
		return QDomDocument{};
	else {
		if(!readFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			fatal("Failed to open xml file for reading with error: " +
				  readFile.errorString().toUtf8());
		}

		QString error;
		int line, column;
		QDomDocument doc;
		// because setContent is not reentrant: lock
		QMutexLocker locker{&_XmlMutex};
		auto ok = doc.setContent(&readFile, &error, &line, &column);
		locker.unlock();
		if(!ok) {
			fatal("Failed to read XML file. Parser error at line" +
				  QByteArray::number(line) + ", column" + QByteArray::number(column) +
				  ": " + readFile.errorString().toUtf8());
		}

		readFile.close();
		return doc;
	}
}

void XmlSyncTask::saveDocument(const QFileInfo &file, const QDomDocument &document)
{
	QSaveFile writeFile{file.absoluteFilePath()};
	if(!writeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		fatal("Failed to open xml file for writing with error: " +
			  writeFile.errorString().toUtf8());
	}
	writeFile.write(document.toByteArray(4));
	if(!writeFile.commit()) {
		fatal("Failed to save written xml file with error: " +
			  writeFile.errorString().toUtf8());
	}
}

QDomDocument XmlSyncTask::createDoc(const QDomDocument &other) const
{
	// set document type
	QDomDocument doc;
	if(!other.doctype().isNull())
		doc = QDomDocument{other.doctype().name()};

	// add xml processing instructions
	auto hasInstr = false;
	auto children = other.childNodes();
	for(auto i = 0; i < children.size(); ++i) {
		if(children.at(i).isProcessingInstruction()) {
			auto proc = children.at(i).toProcessingInstruction();
			if(proc.target() == QStringLiteral("xml")) {
				doc.appendChild(doc.createProcessingInstruction(proc.target(), proc.data()));
				hasInstr = true;
				break;
			}
		}
	}
	if(!hasInstr) {
		doc.appendChild(doc.createProcessingInstruction(QStringLiteral("xml"),
														QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"")));
	}

	// create root element
	doc.appendChild(doc.createElement(other.documentElement().tagName()));
	return doc;
}

QDomElement XmlSyncTask::cd(QDomElement current, const QString &tag, bool &exists) const
{
	auto child = current.firstChildElement(tag);
	if(child.isNull()) {
		child = current.ownerDocument().createElement(tag);
		current.appendChild(child);
		exists = false;
	}
	return child;
}

QString XmlSyncTask::getText(const QDomElement &element) const
{
	// combine text from all direct children
	QString text;
	auto children = element.childNodes();
	for(auto i = 0; i < children.size(); ++i) {
		if(children.at(i).isText())
			text.append(children.at(i).toText().data());
	}
	return text;
}

void XmlSyncTask::writeText(QDomElement element, const QString &text) const
{
	auto children = element.childNodes();
	// remove all old texts
	for(auto i = 0; i < children.size();) {
		if(children.at(i).isText())
			element.removeChild(children.at(i));
		else
			++i;
	}
	// append the new text
	auto txtNode = element.ownerDocument().createTextNode(text);
	element.appendChild(txtNode);
}

void XmlSyncTask::removeAttribs(QDomElement node) const
{
	auto nodes = node.attributes();
	for(auto i = 0; i < nodes.size();) {
		node.removeAttributeNode(nodes.item(i).toAttr());
	}
}
