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

    KMCSECond(RE::BGSSoundDescriptorForm *s, RE::ActorHandle p) { 
        sd = s;
        SEPoint = p;
    }

public:

    RE::BGSSoundDescriptorForm *sd;
    RE::ActorHandle SEPoint;
};

class KMCFSoundDescription {
public:
    KMCFSoundDescription() {}
    KMCFSoundDescription(std::vector<std::pair<int, RE::BGSSoundDescriptorForm *>> sd,
                         std::map<int, std::map<std::string, KMCSECond>> sse) {
        sd_map = sd;
        sd_se_map = sse;
    }

    ~KMCFSoundDescription() {}

public:
    std::vector<std::pair<int, RE::BGSSoundDescriptorForm *>> sd_map;
    std::map<int, std::map<std::string, KMCSECond>> sd_se_map;
};

namespace KMCCT {

    static const int PLAY_SE_PLAYER_POS = 0;
    static const int PLAY_SE_FOLLOWER_POS = 1;

    class KMCSound {
        SINGLETONHEADER(KMCSound)
    public:
        void Init();

        void PlayEx(int trackid, float volume, RE::TESObjectREFR *target, int frand);

        bool GetFirstSEIndexEx(int trackid, int frand, std::string *fstrecord);
        bool PlaySEEx(int trackid, int frand, std::string *record, float volume);
        bool IsPlayableSoundEx(int trackid, int frand);
        void PlayProfileSE(KMCProfileSEType trackid, float volume, RE::TESObjectREFR *target);

    private:
        void Play(int trackid, float volume, RE::TESObjectREFR *target, int frand);
        void Play(int trackid, float volume, RE::TESObjectREFR *target);
        std::string GetFirstSEIndex(int trackid, int frand);
        std::string GetFirstSEIndex(int trackid);
        std::string PlaySE(int trackid, int frand, std::string record, std::string findid, float volume);
        std::string PlaySE(int trackid, std::string record, std::string findid, float volume);
        bool IsPlayableSound(int trackid, int frand);
        bool IsPlayableSound(int trackid);

    private:
        const std::string SD_EDITORID_NAME1 = "aaaKimachuuCutInVF";
        const std::string SD_EDITORID_NAME2 = "SD";

        std::vector<std::pair<int, RE::BGSSoundDescriptorForm *>> sound_descriptor_map;
        std::map<int, std::map<std::string, KMCSECond>> sound_descriptor_se_map;
        std::vector<std::pair<std::string, size_t>> SEIndex;

        std::vector<std::pair<int, KMCFSoundDescription>> FSoundDescriptiorMap;

        // Profile Sound Effect
        std::map<int, RE::BGSSoundDescriptorForm *> ProfileSE;

        RE::BSAudioManager* AudioManager;
    };
}