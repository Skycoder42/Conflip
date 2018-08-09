#include "dconfaccess.h"

extern "C" {
#include <dconf.h>
}

namespace {

void variantCleanup(gpointer data)
{
	delete static_cast<QByteArray*>(data);
}

}

DConfAccess::DConfAccess() :
	_client(dconf_client_new()),
	_path(),
	_needsSync(false)
{}

DConfAccess::~DConfAccess()
{
	sync();
	g_object_unref(_client);
}

void DConfAccess::open(const QByteArray &path)
{
	_path = path;
}

QByteArrayList DConfAccess::readAllKeys(const QByteArrayList &filters) const
{
	auto bList = readAllKeys(QByteArray());
	for(auto it = bList.begin(); it != bList.end();) {
		auto erase = true;
		for(const auto &filter : filters) {
			if(it->startsWith(filter)) {
				erase = false;
				break;
			}
		}
		if(erase)
			it = bList.erase(it);
		else
			it++;
	}
	return bList;
}


std::tuple<QByteArray, QByteArray> DConfAccess::readData(const QByteArray &key) const
{
	// read variant
	auto variant = dconf_client_read(_client, QByteArray(_path + key).constData());
	if(!variant)
		return {};

	// normalize
	auto nVariant = g_variant_get_normal_form(variant);
	g_variant_unref(variant);
	if(!nVariant)
		return {};

	// read type and value
	QByteArray type {g_variant_get_type_string(nVariant)};
	auto size = g_variant_get_size(nVariant);
	QByteArray value(static_cast<int>(size), '\0');
	g_variant_store(nVariant, value.data());
	g_variant_unref(nVariant);

	return {type, value};
}

bool DConfAccess::writeData(const QByteArray &key, const QByteArray &type, const QByteArray &data, QByteArray *errorMsg)
{
	QByteArray realPath = _path + key;

	// create the type
	if(!g_variant_type_string_is_valid(type.constData()))
		return false;
	auto vType = g_variant_type_new(type.constData());

	// create the variant
	auto variant = g_variant_new_from_data(vType,
										   data.constData(), static_cast<gsize>(data.size()),
										   true,
										   variantCleanup, new QByteArray(data));

	//write the value (async)
	GError *error = nullptr;
	auto ok = dconf_client_write_fast(_client, realPath.constData(), variant, &error);
	if(!ok) {
		if(errorMsg)
			*errorMsg = error->message;
		g_error_free(error);
	}

	_needsSync = true;
	g_variant_type_free(vType);
	return ok;
}

void DConfAccess::sync()
{
	if(_needsSync) {
		dconf_client_sync(_client);
		_needsSync = false;
	}
}

QByteArrayList DConfAccess::readAllKeys(const QByteArray &base) const
{
	QByteArray realPath = _path + base;
	gint len = 0;
	auto list = dconf_client_list(_client, realPath.constData(), &len);
	if(list) {
		QByteArrayList bList;
		bList.reserve(len);
		for(gint i = 0; i < len; i++) {
			QByteArray element = base + QByteArray(list[i]);
			if(element.endsWith('/'))
				bList.append(readAllKeys(element));
			else
				bList.append(element);
		}
		g_strfreev(list);
		return bList;
	} else
		return {};
}
