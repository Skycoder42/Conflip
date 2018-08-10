#include "pathresolver.h"

#include <QStandardPaths>

PathResolver::PathResolver(QObject *parent) :
	QObject(parent),
	_scanHidden(true)
{}

QStringList PathResolver::resolvePath(const SyncEntry &entry, SyncHelper *helper) const
{
	_scanHidden = entry.includeHidden;
	_caseSensitive = entry.caseSensitive;

	QStringList allPaths;
	const auto cd = createDir(QStringLiteral("/"));

	// match all "src" paths
	allPaths.append(findFiles(cd, entry.pathPattern.split(QLatin1Char('/'), QString::SkipEmptyParts)));

	// match all "sync" paths
	const auto sPaths = findFiles(cd, helper->toSyncPath(entry.pathPattern).split(QLatin1Char('/'), QString::SkipEmptyParts));
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

QDir PathResolver::createDir(const QString &path) const
{
	QDir dir(path);
	QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Readable;
	if(_caseSensitive)
		filters |= QDir::CaseSensitive;
	if(_scanHidden)
		filters |= QDir::Hidden;
	dir.setFilter(filters);
	return dir;
}
