// code extracted from Ardour

#ifndef URID_MAP
#define URID_MAP

#include <map>
#include <string>
#include <stdint.h>

#include <lv2.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/uri-map/uri-map.h>

class URIDMap
{
public:
    URIDMap();

    LV2_Feature uri_map_feature;
    LV2_URI_Map_Feature uri_map_feature_data;
    LV2_URID_Map urid_map_feature_data;
    LV2_URID_Unmap urid_unmap_feature_data;
    LV2_Feature urid_map_feature;
    LV2_Feature urid_unmap_feature;

    const char* id_to_uri(const uint32_t id);
    uint32_t uri_to_id(const char* uri);

    typedef std::map<const std::string, uint32_t> Map;
    typedef std::map<uint32_t, const std::string> Unmap;

    Map map;
    Unmap unmap;
};

#endif
