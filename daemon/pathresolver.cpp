#include "pathresolver.h"

#include <QStandardPaths>

PathResolver::PathResolver(QObject *parent) :
	QObject(parent),
	_scanHidden(true)
{}

QStringList PathResolver::resolvePath(const SyncEntry &entry) const
{
	auto pathList = entry.pathPattern.split(QLatin1Char('/'), QString::SkipEmptyParts);
	_scanHidden = entry.includeHidden;
	auto cd = createDir(QStringLiteral("/"));
	return findFiles(cd, pathList);
}

QStringList PathResolver::findFiles(const QDir &cd, QStringList pathList) const
{
	auto element = pathList.takeFirst();
	if(element == QStringLiteral("**")) {
		QStringList expList {element};
		expList.append(pathList);

		QStringList resList;
		auto subElements = cd.entryInfoList();
		for(auto subElement : subElements) {
			if(!subElement.isDir())
				continue;
			// search without **
			resList.append(findFiles(createDir(subElement.absoluteFilePath()), pathList));
			// search with **
			resList.append(findFiles(createDir(subElement.absoluteFilePath()), expList));
		}

		return resList;
	} else if(element == QLatin1Char('~')) {
		auto home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
		return findFiles(createDir(home), pathList);
	} else {
		QStringList resList;

		auto subElements = cd.entryInfoList({element});
		if(pathList.isEmpty()) {
			for(auto subElement : subElements)
				resList.append(subElement.absoluteFilePath());
		} else {
			for(auto subElement : subElements) {
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
