
#include "urid_map.h"

#define UNUSED_PARAM(var) do { (void)(var); } while (0)

static uint32_t g_uri_map_uri_to_id(LV2_URI_Map_Callback_Data callback_data, const char* map, const char* uri)
{
    UNUSED_PARAM(map);
    URIDMap* const me = (URIDMap*)callback_data;
    const uint32_t id = me->uri_to_id(uri);
    return id;
}

static LV2_URID g_urid_map(LV2_URID_Map_Handle handle, const char* uri)
{
    URIDMap* const me = (URIDMap*)handle;
    return me->uri_to_id(uri);
}

static const char* g_urid_unmap(LV2_URID_Unmap_Handle handle, LV2_URID urid)
{
    URIDMap* const me = (URIDMap*)handle;
    return me->id_to_uri(urid);
}

URIDMap::URIDMap()
{
    uri_map_feature_data.uri_to_id = g_uri_map_uri_to_id;
    uri_map_feature_data.callback_data = this;
    uri_map_feature.URI = LV2_URI_MAP_URI;
    uri_map_feature.data = &uri_map_feature_data;

    urid_map_feature_data.map = g_urid_map;
    urid_map_feature_data.handle = this;
    urid_map_feature.URI = LV2_URID_MAP_URI;
    urid_map_feature.data = &urid_map_feature_data;

    urid_unmap_feature_data.unmap = g_urid_unmap;
    urid_unmap_feature_data.handle = this;
    urid_unmap_feature.URI = LV2_URID_UNMAP_URI;
    urid_unmap_feature.data = &urid_unmap_feature_data;
}

uint32_t URIDMap::uri_to_id(const char* uri)
{
    const std::string urimm(uri);
    const Map::const_iterator i = map.find(urimm);
    if (i != map.end()) return i->second;

    const uint32_t id = map.size() + 1;
    map.insert(std::make_pair(urimm, id));
    unmap.insert(std::make_pair(id, urimm));
    return id;
}

const char* URIDMap::id_to_uri(const uint32_t id)
{
    const Unmap::const_iterator i = unmap.find(id);
    return (i != unmap.end()) ? i->second.c_str() : NULL;
}