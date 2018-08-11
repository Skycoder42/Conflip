#include "pathresolver.h"

#include <QStandardPaths>
#include <QStorageInfo>

PathResolver::PathResolver(QObject *parent) :
	QObject(parent),
	_scanHidden(true)
{}

QStringList PathResolver::resolvePath(const SyncEntry &entry, SyncHelper *helper) const
{
	_scanHidden = entry.includeHidden;
	_caseSensitive = entry.caseSensitive;

	QStringList allPaths;

	// match all "src" paths
	auto pathList = entry.pathPattern.split(QLatin1Char('/'), QString::SkipEmptyParts);
	auto cd = findRootDir(pathList);
	allPaths.append(findFiles(cd, pathList));

	// match all "sync" paths
	pathList = helper->toSyncPath(entry.pathPattern).split(QLatin1Char('/'), QString::SkipEmptyParts);
	cd = findRootDir(pathList);
	const auto sPaths = findFiles(cd, pathList);
	allPaths.reserve(allPaths.size() + sPaths.size());
	for(const auto &path : sPaths)
		allPaths.append(helper->toSrcPath(path));

	allPaths.removeDuplicates();
	return allPaths;
}

QStringList PathResolver::findFiles(const QDir &cd, QStringList pathList) const
{
	auto element = pathList.takeFirst();
	if(element == QStringLiteral("**")) {
		QStringList expList {element};
		expList.append(pathList);

		QStringList resList;
		auto subElements = cd.entryInfoList();
		for(const auto& subElement : subElements) {
			if(!subElement.isDir())
				continue;
			// search without **
			resList.append(findFiles(createDir(subElement.absoluteFilePath()), pathList));
			// search with **
			resList.append(findFiles(createDir(subElement.absoluteFilePath()), expList));
		}

		return resList;
	} else {
		QStringList resList;
		auto subElements = cd.entryInfoList({element});
		resList.reserve(subElements.size());
		if(pathList.isEmpty()) {
			for(const auto& subElement : subElements)
				resList.append(subElement.absoluteFilePath());
		} else {
			for(const auto& subElement : subElements) {
				if(!subElement.isDir())
					continue;
				resList.append(findFiles(createDir(subElement.absoluteFilePath()), pathList));
			}
		}

		return resList;
	}
}

QDir PathResolver::findRootDir(QStringList &pathList) const
{
	if(pathList.isEmpty())
		return QDir::root();

	const QDir firstDir{pathList.first()};
	const QStorageInfo firstInfo{pathList.first()};
	if(firstInfo.isValid() && firstInfo.rootPath() == firstDir.absolutePath()) { //NOTE untested
		pathList.takeFirst();
		return firstDir;
	} else
	   return QDir::root();
}

QDir PathResolver::createDir(const QString &path) const
{
	QDir dir{path};
	QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Readable;
	if(_caseSensitive)
		filters |= QDir::CaseSensitive;
	if(_scanHidden)
		filters |= QDir::Hidden;
	dir.setFilter(filters);
	return dir;
}
