#include "pathresolver.h"

#include <QStandardPaths>
#include <QStorageInfo>

PathResolver::PathResolver(QObject *parent) :
	QObject(parent)
{}

QStringList PathResolver::resolvePath(const SyncEntry &entry, SyncHelper *helper) const
{
	QStringList allPaths;

	// match all "src" paths
	auto pathList = entry.pathPattern.split(QLatin1Char('/'), QString::SkipEmptyParts);
	auto cd = findRootDir(pathList, entry);
	allPaths.append(findFiles(cd, pathList, entry));

	// match all "sync" paths
	pathList = helper->toSyncPath(entry.pathPattern).split(QLatin1Char('/'), QString::SkipEmptyParts);
	cd = findRootDir(pathList, entry);
	const auto sPaths = findFiles(cd, pathList, entry);
	allPaths.reserve(allPaths.size() + sPaths.size());
	for(const auto &path : sPaths)
		allPaths.append(helper->toSrcPath(path));

	allPaths.removeDuplicates();
	return allPaths;
}

QStringList PathResolver::findFiles(const QDir &cd, QStringList pathList, const SyncEntry &entry) const
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
			resList.append(findFiles(createDir(subElement.absoluteFilePath(), entry), pathList, entry));
			// search with **
			resList.append(findFiles(createDir(subElement.absoluteFilePath(), entry), expList, entry));
		}

		return resList;
	} else {
		QStringList resList;
		auto subElements = cd.entryInfoList({element});
		resList.reserve(subElements.size());
		if(pathList.isEmpty()) {
			for(const auto& subElement : subElements) {
				if(!entry.matchDirs && subElement.isDir())
					continue;
				resList.append(subElement.absoluteFilePath());
			}
		} else {
			for(const auto& subElement : subElements) {
				if(!subElement.isDir())
					continue;
				resList.append(findFiles(createDir(subElement.absoluteFilePath(), entry), pathList, entry));
			}
		}

		return resList;
	}
}

QDir PathResolver::findRootDir(QStringList &pathList, const SyncEntry &entry) const
{
	if(pathList.isEmpty())
		return createDir(QDir::rootPath(), entry);

	const QStorageInfo firstInfo{pathList.first()};
	if(firstInfo.isValid() && firstInfo.rootPath() == pathList.first()) //NOTE untested on win
		return createDir(pathList.takeFirst(), entry);
	else
		return createDir(QDir::rootPath(), entry);
}

QDir PathResolver::createDir(const QString &path, const SyncEntry &entry) const
{
	QDir dir{path};
	QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Readable;
	if(entry.caseSensitive)
		filters |= QDir::CaseSensitive;
	if(entry.includeHidden)
		filters |= QDir::Hidden;
	dir.setFilter(filters);
	return dir;
}
