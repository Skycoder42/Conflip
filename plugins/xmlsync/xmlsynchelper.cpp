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

void XmlSyncHelper::performSync(const QString &path, const QString &mode, const QStringList &extras, bool isFirstUse)
{
	if(mode != ModeXml)
		throw SyncException("Unsupported path mode");

	QFileInfo srcInfo, syncInfo;
	std::tie(srcInfo, syncInfo) = generatePaths(QStringLiteral("xml"), path);

	// step 1: load the two xml models
	auto srcDoc = loadDocument(srcInfo);
	auto syncDoc = loadDocument(syncInfo);
	bool srcNeedsUpdate = false;
	bool syncNeedsUpdate = false;
	setupRootElements(srcDoc, syncDoc);
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
						   tree,
						   srcInfo);
				lastElemNeeded = false;
				break; //chain must end here
			// handle attribute content (all attributes)
			} else if(link == QStringLiteral("=")) {
				updateAttributes(srcElement, syncElement,
								 srcIsNewer,
								 srcNeedsUpdate, syncNeedsUpdate,
								 tree,
								 srcInfo);
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
								tree,
								srcInfo);
				lastElemNeeded = false;
				break; //chain must end here
			// handle text content and all attributes
			} else if(link == QStringLiteral("#")) {
				updateText(srcElement, syncElement,
						   srcExists, syncExists,
						   srcIsNewer,
						   srcNeedsUpdate, syncNeedsUpdate,
						   tree,
						   srcInfo);
				updateAttributes(srcElement, syncElement,
								 srcIsNewer,
								 srcNeedsUpdate, syncNeedsUpdate,
								 tree,
								 srcInfo);
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
					log(srcInfo, "Updated nodes in sync from src", tree);
				} else {
					auto srcParent = srcElement.parentNode();
					auto newChild = syncElement.cloneNode(true);
					if(!includeAttribs)
						removeAttribs(newChild.toElement());
					srcParent.replaceChild(newChild, srcElement);
					log(srcInfo, "Updated nodes in src from sync", tree);
				}
			} else if(srcExists) {
				auto syncParent = syncElement.parentNode();
				auto newChild = srcElement.cloneNode(true);
				if(!includeAttribs)
					removeAttribs(newChild.toElement());
				syncParent.replaceChild(newChild, syncElement);
				log(srcInfo, "Added new nodes in sync from src", tree);
			} else if(syncExists) {
				auto srcParent = srcElement.parentNode();
				auto newChild = syncElement.cloneNode(true);
				if(!includeAttribs)
					removeAttribs(newChild.toElement());
				srcParent.replaceChild(newChild, srcElement);
				log(srcInfo, "Added new nodes in src from sync", tree);
			}
		}
	}

	// step 3: write the files
	if(srcNeedsUpdate)
		saveDocument(srcInfo, srcDoc);
	if(syncNeedsUpdate)
		saveDocument(syncInfo, syncDoc);
}

void XmlSyncHelper::undoSync(const QString &path, const QString &mode)
{
	if(mode != ModeXml)
		throw SyncException("Unsupported path mode");
	removeSyncPath(QStringLiteral("xml"), path, "XML-SYNC");
}

void XmlSyncHelper::updateText(const QDomElement& srcElement, const QDomElement& syncElement, bool srcExists, bool syncExists, bool srcIsNewer, bool &srcNeedsUpdate, bool &syncNeedsUpdate, const QString &key, const QFileInfo &srcInfo)
{
	// both have the text
	if(srcExists && syncExists) {
		auto srcText = srcElement.text();
		auto syncText = syncElement.text();
		// text is different
		if(srcText != syncText) {
			if(srcIsNewer) {
				writeText(syncElement, srcText);
				syncNeedsUpdate = true;
				log(srcInfo, "Updated text in sync from src", key);
			} else {
				writeText(srcElement, syncText);
				srcNeedsUpdate = true;
				log(srcInfo, "Updated text in src from sync", key);
			}
		} else
			log(srcInfo, "Skipping unchanged text", key, true);
	// only src has text
	} else if(srcExists) {
		writeText(syncElement, srcElement.text());
		syncNeedsUpdate = true;
		log(srcInfo, "Added new text in sync from src", key);
	// only sync has text
	} else if(syncExists) {
		writeText(srcElement, syncElement.text());
		srcNeedsUpdate = true;
		log(srcInfo, "Added new text in src from sync", key);
	}
}

void XmlSyncHelper::updateAttributes(const QDomElement& srcElement, const QDomElement& syncElement, bool srcIsNewer, bool &srcNeedsUpdate, bool &syncNeedsUpdate, const QString &key, const QFileInfo &srcInfo)
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
						key + name,
						srcInfo);
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
						key + name,
						srcInfo);
	}
}

void XmlSyncHelper::updateAttribute(QDomElement srcElement, QDomElement syncElement, QDomAttr srcAttr, QDomAttr syncAttr, bool srcIsNewer, bool &srcNeedsUpdate, bool &syncNeedsUpdate, const QString &key, const QFileInfo &srcInfo)
{
	// both have attrib
	if(!srcAttr.isNull() && !syncAttr.isNull()) {
		auto srcValue = srcAttr.value();
		auto syncValue = syncAttr.value();
		if(srcValue != syncValue) {
			if(srcIsNewer) {
				syncAttr.setValue(srcValue);
				syncNeedsUpdate = true;
				log(srcInfo, "Updated attribute in sync from src", key);
			} else {
				srcAttr.setValue(syncValue);
				srcNeedsUpdate = true;
				log(srcInfo, "Updated attribute in src from sync", key);
			}
		} else
			log(srcInfo, "Skipping unchanged attribute", key, true);
	// only src has attrib
	} else if(!srcAttr.isNull()) {
		syncAttr = syncElement.ownerDocument().createAttribute(srcAttr.name());
		syncAttr.setValue(srcAttr.value());
		syncElement.setAttributeNode(syncAttr);
		syncNeedsUpdate = true;
		log(srcInfo, "Added new attribute in sync from src", key);
	// only sync has attrib
	} else if(!syncAttr.isNull()) {
		srcAttr = srcElement.ownerDocument().createAttribute(syncAttr.name());
		srcAttr.setValue(syncAttr.value());
		srcElement.setAttributeNode(srcAttr);
		srcNeedsUpdate = true;
		log(srcInfo, "Added new attribute in src from sync", key);
	}
}

QDomDocument XmlSyncHelper::loadDocument(const QFileInfo &file) const
{
	QFile readFile(file.absoluteFilePath());
	if(!readFile.exists())
		return QDomDocument();
	else {
		if(!readFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			throw SyncException("Failed to open xml file for reading with error: " +
								readFile.errorString().toUtf8());
		}

		QString error;
		int line, column;
		QDomDocument doc;
		if(!doc.setContent(&readFile, &error, &line, &column)) {
			throw SyncException("Failed to read XML file. Parser error at line" +
								QByteArray::number(line) + ", column" + QByteArray::number(column) +
								": " + readFile.errorString().toUtf8());
		}

		readFile.close();
		return doc;
	}
}

void XmlSyncHelper::saveDocument(const QFileInfo &file, const QDomDocument& document)
{
	QSaveFile writeFile(file.absoluteFilePath());
	if(!writeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		throw SyncException("Failed to open xml file for writing with error: " +
							writeFile.errorString().toUtf8());
	}
	writeFile.write(document.toByteArray(4));
	if(!writeFile.commit()) {
		throw SyncException("Failed to save written xml file with error: " +
							writeFile.errorString().toUtf8());
	}
}

void XmlSyncHelper::setupRootElements(QDomDocument &srcDoc, QDomDocument &syncDoc) const
{
	if(srcDoc.documentElement().isNull()) {
		if(!syncDoc.doctype().isNull())
			srcDoc = QDomDocument(syncDoc.doctype().name());
		auto root = srcDoc.createElement(syncDoc.documentElement().tagName());
		srcDoc.appendChild(root);
	} else if(syncDoc.documentElement().isNull()) {
		if(!srcDoc.doctype().isNull())
			syncDoc = QDomDocument(srcDoc.doctype().name());
		auto root = syncDoc.createElement(srcDoc.documentElement().tagName());
		syncDoc.appendChild(root);
	}
}

QDomElement XmlSyncHelper::cd(QDomElement current, const QString &tag, bool &exists) const
{
	auto child = current.firstChildElement(tag);
	if(child.isNull()) {
		child = current.ownerDocument().createElement(tag);
		current.appendChild(child);
		exists = false;
	}
	return child;
}

void XmlSyncHelper::writeText(QDomElement element, const QString &text) const
{
	auto children = element.childNodes();
	for(auto i = 0; i < children.size(); i++) {
		if(children.at(i).isText()) {
			auto txtNode = children.at(i).toText();
			txtNode.setData(text);
			return;
		}
	}

	auto txtNode = element.ownerDocument().createTextNode(text);
	element.appendChild(txtNode);
}

void XmlSyncHelper::removeAttribs(QDomElement node) const
{
	auto nodes = node.attributes();
	for(auto i = 0; i < nodes.size();) {
		node.removeAttributeNode(nodes.item(i).toAttr());
	}
}

void XmlSyncHelper::log(const QFileInfo &file, const char *msg, bool dbg) const
{
	(dbg ? qDebug() : qInfo()).noquote() << "XML-SYNC:" << file.absoluteFilePath() << "=>" << msg;
}

void XmlSyncHelper::log(const QFileInfo &file, const char *msg, const QString &key, bool dbg) const
{
	(dbg ? qDebug() : qInfo()).noquote() << "XML-SYNC:" << file.absoluteFilePath()
										 << "=>" << msg << (QLatin1Char('[') + key + QLatin1Char(']'));
}
