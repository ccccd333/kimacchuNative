#pragma once
#include "KMCUtility.h"

#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

struct KMCSECond {
public:
    KMCSECond() {}

    KMCSECond(RE::BGSSoundDescriptorForm *s, RE::Actor *p) { 
        sd = s;
        SEPoint = p;
    }

public:

    RE::BGSSoundDescriptorForm *sd;
    RE::Actor *SEPoint;
};

class KMCFSoundDescription {
public:
    KMCFSoundDescription() {}
    KMCFSoundDescription(std::vector<std::pair<std::string, RE::BGSSoundDescriptorForm *>> sd,
                         std::map<std::string, std::map<std::string, KMCSECond>> sse,
                         std::vector<std::pair<std::string, size_t>> si) {
        SoundDescriptorMap = sd;
        SDSEMap = sse;
        SEIndex = si;
    }

    ~KMCFSoundDescription() {}

public:
    std::vector<std::pair<std::string, RE::BGSSoundDescriptorForm *>> SoundDescriptorMap;
    std::map<std::string, std::map<std::string, KMCSECond>> SDSEMap;
    std::vector<std::pair<std::string, size_t>> SEIndex;
};

namespace KMCCT {

    static const int PLAY_SE_PLAYER_POS = 0;
    static const int PLAY_SE_FOLLOWER_POS = 1;

    class KMCSound {
        SINGLETONHEADER(KMCSound)
    public:
        void Init();

        void PlayEx(std::string trackid, float volume, RE::TESObjectREFR *target, int frand);

        bool GetFirstSEIndexEx(std::string trackid, int frand, std::string *fstrecord);
        bool PlaySEEx(std::string trackid, int frand, std::string *record, float volume);
        bool IsPlayableSoundEx(std::string trackid, int frand);
        void PlayProfileSE(KMCProfileSEType trackid, float volume, RE::TESObjectREFR *target);

    private:
        void Play(std::string trackid, float volume, RE::TESObjectREFR *target, int frand);
        void Play(std::string trackid, float volume, RE::TESObjectREFR *target);
        std::string GetFirstSEIndex(std::string trackid, int frand);
        std::string GetFirstSEIndex(std::string trackid);
        std::string PlaySE(std::string trackid, int frand, std::string record, std::string findid, float volume);
        std::string PlaySE(std::string trackid, std::string record, std::string findid, float volume);
        bool IsPlayableSound(std::string trackid, int frand);
        bool IsPlayableSound(std::string trackid);

    private:
        const std::string SD_EDITORID_NAME1 = "aaaKimachuuCutInVF";
        const std::string SD_EDITORID_NAME2 = "SD";

        std::vector<std::pair<std::string, RE::BGSSoundDescriptorForm *>> SoundDescriptorMap;
        std::map<std::string, std::map<std::string, KMCSECond>> SoundDescriptorSEMap;
        std::vector<std::pair<std::string, size_t>> SEIndex;

        std::vector<std::pair<int, KMCFSoundDescription>> FSoundDescriptiorMap;

        // Profile Sound Effect
        std::map<int, RE::BGSSoundDescriptorForm *> ProfileSE;

        RE::BSAudioManager* AudioManager;
    };
}