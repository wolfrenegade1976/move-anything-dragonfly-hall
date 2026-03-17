#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include "DistrhoPluginInfo.h"
#include "DSP.hpp"

typedef struct host_api_v1 {} host_api_v1_t;
typedef struct audio_fx_api_v2 {
    uint32_t api_version;
    void* (*create_instance)(const char*, const char*);
    void  (*destroy_instance)(void*);
    void  (*process_block)(void*, int16_t*, int);
    void  (*set_param)(void*, const char*, const char*);
    int   (*get_param)(void*, const char*, char*, int);
    void  (*on_midi)(void*, const uint8_t*, int, int);
} audio_fx_api_v2_t;

struct HallPreset { const char *name; int bank; int preset; };
static const HallPreset PRESETS[] = {
    {"Bright Rm",   0,0},{"Clear Rm",    0,1},{"Dark Rm",     0,2},
    {"Sm Chamber",  0,3},{"Lg Chamber",  0,4},
    {"Acoustic St", 1,0},{"Electric St", 1,1},{"Perc St",     1,2},
    {"Piano St",    1,3},{"Vocal St",    1,4},
    {"Sm Bright",   2,0},{"Sm Clear",    2,1},{"Sm Dark",     2,2},
    {"Sm Perc",     2,3},{"Sm Vocal",    2,4},
    {"Med Bright",  3,0},{"Med Clear",   3,1},{"Med Dark",    3,2},
    {"Med Perc",    3,3},{"Med Vocal",   3,4},
    {"Lg Bright",   4,0},{"Lg Clear",    4,1},{"Lg Dark",     4,2},
    {"Lg Vocal",    4,3},{"Great Hall",  4,4},
};
static const int NUM_PRESETS = 25;

struct HallInst { DragonflyReverbDSP *dsp; float p[paramCount]; int cur; };

static void load_preset(HallInst *h, int idx) {
    if (idx < 0 || idx >= NUM_PRESETS) return;
    h->cur = idx;
    const float *p = banks[PRESETS[idx].bank].presets[PRESETS[idx].preset].params;
    for (int i = 0; i < paramCount; i++) { h->p[i] = p[i]; h->dsp->setParameterValue(i, p[i]); }
}

static void* hall_create(const char*, const char*) {
    HallInst *h = new HallInst();
    h->dsp = new DragonflyReverbDSP(44100.0);
    load_preset(h, 10);
    return h;
}
static void hall_destroy(void *inst) { HallInst *h=(HallInst*)inst; delete h->dsp; delete h; }

static void hall_process(void *inst, int16_t *audio, int frames) {
    HallInst *h = (HallInst*)inst;
    const int B = 128;
    float iL[B],iR[B],oL[B],oR[B];
    for (int off=0; off<frames;) {
        int n = frames-off; if(n>B) n=B;
        for(int i=0;i<n;i++){iL[i]=audio[(off+i)*2]/32768.f;iR[i]=audio[(off+i)*2+1]/32768.f;}
        const float*ins[2]={iL,iR}; float*outs[2]={oL,oR};
        h->dsp->run(ins,outs,(uint32_t)n);
        for(int i=0;i<n;i++){
            float l=oL[i],r=oR[i];
            if(l>1)l=1;if(l<-1)l=-1;if(r>1)r=1;if(r<-1)r=-1;
            audio[(off+i)*2]=(int16_t)(l*32767);audio[(off+i)*2+1]=(int16_t)(r*32767);
        }
        off+=n;
    }
}

static int key_to_param(const char *k) {
    for(int i=0;i<paramCount;i++) if(strcmp(PARAMS[i].symbol,k)==0) return i;
    return -1;
}

static void hall_set_param(void *inst, const char *key, const char *val) {
    HallInst *h=(HallInst*)inst;
    if(strcmp(key,"preset")==0) {
        int idx=atoi(val);
        if(idx>=0&&idx<NUM_PRESETS) load_preset(h,idx);
        return;
    }
    // Restore full state from JSON snapshot
    if(strcmp(key,"set_state")==0) {
        const char *p = strstr(val,"\"preset\":");
        if(p) { int idx=atoi(p+9); if(idx>=0&&idx<NUM_PRESETS) h->cur=idx; }
        for(int i=0;i<paramCount;i++) {
            char search[64];
            snprintf(search,sizeof(search),"\"%s\":",PARAMS[i].symbol);
            const char *found=strstr(val,search);
            if(found) {
                float v=(float)atof(found+strlen(search));
                h->p[i]=v; h->dsp->setParameterValue(i,v);
            }
        }
        return;
    }
    int idx=key_to_param(key); if(idx<0) return;
    h->p[idx]=(float)atof(val); h->dsp->setParameterValue(idx,h->p[idx]);
}

static int hall_get_param(void *inst, const char *key, char *buf, int buf_len) {
    HallInst *h=(HallInst*)inst;

    if(strcmp(key,"preset")==0)
        return snprintf(buf,buf_len,"%d",h->cur);
    if(strcmp(key,"preset_count")==0)
        return snprintf(buf,buf_len,"%d",NUM_PRESETS);
    if(strcmp(key,"preset_name")==0)
        return snprintf(buf,buf_len,"%s",PRESETS[h->cur].name);

    // State snapshot for patch persistence
    if(strcmp(key,"get_state")==0) {
        int n=snprintf(buf,buf_len,"{\"preset\":%d",h->cur);
        for(int i=0;i<paramCount&&n<buf_len-32;i++)
            n+=snprintf(buf+n,buf_len-n,",\"%s\":%.4f",PARAMS[i].symbol,h->p[i]);
        if(n<buf_len-2) { buf[n]='}'; buf[n+1]='\0'; n++; }
        return n;
    }

    if(strcmp(key,"ui_hierarchy")==0) {
        static const char j[]=
            "{\"modes\":null,\"levels\":{\"root\":{"
            "\"label\":\"Dragonfly Hall\","
            "\"list_param\":\"preset\","
            "\"count_param\":\"preset_count\","
            "\"name_param\":\"preset_name\","
            "\"children\":null,"
            "\"knobs\":[\"dry_level\",\"early_level\",\"late_level\",\"size\",\"decay\",\"diffuse\",\"delay\",\"width\"],"
            "\"params\":["
            "{\"key\":\"dry_level\",\"label\":\"Dry\"},"
            "{\"key\":\"early_level\",\"label\":\"Early\"},"
            "{\"key\":\"late_level\",\"label\":\"Late\"},"
            "{\"key\":\"size\",\"label\":\"Size\"},"
            "{\"key\":\"decay\",\"label\":\"Decay\"},"
            "{\"key\":\"diffuse\",\"label\":\"Diffuse\"},"
            "{\"key\":\"delay\",\"label\":\"Predelay\"},"
            "{\"key\":\"width\",\"label\":\"Width\"},"
            "{\"key\":\"modulation\",\"label\":\"Modulation\"},"
            "{\"key\":\"wander\",\"label\":\"Wander\"},"
            "{\"key\":\"high_cut\",\"label\":\"High Cut\"},"
            "{\"key\":\"high_xo\",\"label\":\"High Cross\"},"
            "{\"key\":\"high_mult\",\"label\":\"High Mult\"},"
            "{\"key\":\"low_cut\",\"label\":\"Low Cut\"},"
            "{\"key\":\"low_xo\",\"label\":\"Low Cross\"},"
            "{\"key\":\"low_mult\",\"label\":\"Low Mult\"}"
            "]}}}";
        strncpy(buf,j,buf_len-1); buf[buf_len-1]='\0'; return strlen(buf);
    }

    if(strcmp(key,"chain_params")==0) {
        static const char j[]=
            "[{\"key\":\"dry_level\",\"name\":\"Dry\",\"type\":\"float\",\"min\":0,\"max\":100,\"step\":1},"
            "{\"key\":\"early_level\",\"name\":\"Early\",\"type\":\"float\",\"min\":0,\"max\":100,\"step\":1},"
            "{\"key\":\"late_level\",\"name\":\"Late\",\"type\":\"float\",\"min\":0,\"max\":100,\"step\":1},"
            "{\"key\":\"size\",\"name\":\"Size\",\"type\":\"float\",\"min\":10,\"max\":60,\"step\":1},"
            "{\"key\":\"decay\",\"name\":\"Decay\",\"type\":\"float\",\"min\":0.1,\"max\":10,\"step\":0.1},"
            "{\"key\":\"diffuse\",\"name\":\"Diffuse\",\"type\":\"float\",\"min\":0,\"max\":100,\"step\":1},"
            "{\"key\":\"delay\",\"name\":\"Predelay\",\"type\":\"float\",\"min\":0,\"max\":100,\"step\":1},"
            "{\"key\":\"width\",\"name\":\"Width\",\"type\":\"float\",\"min\":50,\"max\":150,\"step\":1},"
            "{\"key\":\"modulation\",\"name\":\"Modulation\",\"type\":\"float\",\"min\":0,\"max\":100,\"step\":1},"
            "{\"key\":\"wander\",\"name\":\"Wander\",\"type\":\"float\",\"min\":0,\"max\":40,\"step\":0.5},"
            "{\"key\":\"high_cut\",\"name\":\"High Cut\",\"type\":\"float\",\"min\":1000,\"max\":16000,\"step\":100},"
            "{\"key\":\"high_xo\",\"name\":\"High Cross\",\"type\":\"float\",\"min\":1000,\"max\":16000,\"step\":100},"
            "{\"key\":\"high_mult\",\"name\":\"High Mult\",\"type\":\"float\",\"min\":0.2,\"max\":1.2,\"step\":0.05},"
            "{\"key\":\"low_cut\",\"name\":\"Low Cut\",\"type\":\"float\",\"min\":0,\"max\":200,\"step\":1},"
            "{\"key\":\"low_xo\",\"name\":\"Low Cross\",\"type\":\"float\",\"min\":200,\"max\":1200,\"step\":10},"
            "{\"key\":\"low_mult\",\"name\":\"Low Mult\",\"type\":\"float\",\"min\":0.5,\"max\":2.5,\"step\":0.05}]";
        strncpy(buf,j,buf_len-1); buf[buf_len-1]='\0'; return strlen(buf);
    }

    int idx=key_to_param(key); if(idx<0) return -1;
    return snprintf(buf,buf_len,"%.4f",h->p[idx]);
}

static void hall_on_midi(void*,const uint8_t*,int,int){}
static audio_fx_api_v2_t g_api={2,hall_create,hall_destroy,hall_process,hall_set_param,hall_get_param,hall_on_midi};
extern "C" audio_fx_api_v2_t *move_audio_fx_init_v2(const host_api_v1_t *host){(void)host;return &g_api;}
