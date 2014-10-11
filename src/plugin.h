
#ifndef PLUGIN_H
#define PLUGIN_H

#include <stdint.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <map>

#include <lilv/lilv.h>
#include "lilvmm.hpp"

#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/time/time.h>
#include <lv2/lv2plug.in/ns/ext/patch/patch.h>
#include <lv2/lv2plug.in/ns/ext/event/event.h>
#include <lv2/lv2plug.in/ns/ext/worker/worker.h>
#include <lv2/lv2plug.in/ns/ext/uri-map/uri-map.h>
#include <lv2/lv2plug.in/ns/ext/options/options.h>
#include <lv2/lv2plug.in/ns/ext/buf-size/buf-size.h>
#include <lv2/lv2plug.in/ns/ext/port-props/port-props.h>
#include <lv2/lv2plug.in/ns/ext/parameters/parameters.h>

#include "urid_map.h"
#include "worker.h"
#include "lv2_evbuf.h"

#define MINIMUM_PRESET_LABEL "minimum"
#define MAXIMUM_PRESET_LABEL "maximum"
#define DEFAULT_PRESET_LABEL "default"

#define FEATURES_COUNT      5
#define EVENT_BUFFER_SIZE   4096

class Plugin;
class PortGroup;

struct param_range_t
{
    float *min, *max, *def;
};

struct scale_point_t
{
    uint32_t count;
    std::vector<const char*> labels;
    std::vector<float> values;
};

struct port_data_t
{
    const char *name, *symbol;
    float value, min, max, def;
    scale_point_t scale_points;

    float *buffer;
    LV2_Evbuf *event_buffer;

    bool is_integer, is_logarithmic, is_enumeration, is_scale_point, is_toggled, is_trigger;

    inline void write_buffer(float *buffer, uint32_t buffer_size=1)
    {
        std::memcpy(this->buffer, buffer, buffer_size * sizeof(float));
    }

    inline void read_buffer(float *buffer, uint32_t buffer_size=1)
    {
        std::memcpy(buffer, this->buffer, buffer_size * sizeof(float));
    }
};

class PortGroup
{
public:
    PortGroup(Plugin* p, Lilv::Node type, uint32_t sample_count = 1);
    ~PortGroup();

    Plugin* plugin;
    std::map<uint32_t,port_data_t> inputs_by_index;
    std::map<uint32_t,port_data_t> outputs_by_index;

    std::map<std::string,port_data_t*> inputs_by_symbol;
    std::map<std::string,port_data_t*> outputs_by_symbol;

    // TODO: set and get of output controls, the below function are only to input
    void set_value(std::string preset);
    void set_value(uint32_t index, float value);
    float get_value(uint32_t index);
};

class Plugin : public Workee
{
public:
    Plugin(std::string uri, uint32_t sample_rate, uint32_t sample_count);
    ~Plugin();

    void run(uint32_t sample_count);

    std::string uri;
    uint32_t sample_rate, sample_count;

    Lilv::Plugin* plugin;
    Lilv::Instance* instance;

    // ports
    PortGroup *audio, *control, *atom;
    param_range_t ranges;
    uint32_t num_ports;

    LV2_Feature** features;

    // urid
    static URIDMap urid_map;
    struct URIDs
    {
        LV2_URID atom_Int;
        LV2_URID atom_Float;
        LV2_URID atom_Chunk;
        LV2_URID atom_Path;
        LV2_URID atom_Sequence;
        LV2_URID atom_eventTransfer;
        LV2_URID midi_MidiEvent;
        LV2_URID time_Position;
        LV2_URID time_bar;
        LV2_URID time_barBeat;
        LV2_URID time_beatUnit;
        LV2_URID time_beatsPerBar;
        LV2_URID time_beatsPerMinute;
        LV2_URID time_frame;
        LV2_URID time_speed;
        LV2_URID bufsize_minBlockLength;
        LV2_URID bufsize_maxBlockLength;
        LV2_URID bufsize_sequenceSize;
        LV2_URID parameters_sampleRate;
    };
    static URIDs urids;

    // options
    LV2_Feature options_feature;
    int32_t seq_size;

    // worker
    LV2_Feature work_schedule_feature;
    const LV2_Worker_Interface* work_iface;
    Worker* worker;
    int work(uint32_t size, const void* data);
    int work_response(uint32_t size, const void* data);
};

#endif
