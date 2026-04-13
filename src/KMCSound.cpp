#include "KMCSound.h"
#include "KMCConfig.h"
#include "KMCDisplayAddon.h"
#include "KMCDisplayWordAndTexture.h"
#include <IWWConfig.h>

SINGLETONBODY(KMCCT::KMCSound)

namespace KMCCT {
    using namespace boost::property_tree;

    void InitLoop(std::vector<std::pair<int, RE::BGSSoundDescriptorForm*>>* target,
                  std::map<int, std::map<std::string, KMCSECond>>* target_sound_effect,
                  const ActorAddonSet* addon_set, RE::Actor* player,
                  RE::Actor* follower) {
        for (const auto& [key, value] : addon_set->cutin_entries) {
            std::string track_id = std::to_string(key);

            if (!value.voice_ref.empty()) {
                auto spvalue = KMCSplit(value.voice_ref, ',');
                if (spvalue.size() == 2) {
                    auto* sd = (RE::BGSSoundDescriptorForm*)RE::TESDataHandler::GetSingleton()->LookupForm(
                        std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));
                    if (!sd) {
                        ERROR("ERROR Possibly wrong definition of SoundDescriptorForm. key: {} FORM ID: {} {}",
                              track_id, spvalue.at(0), spvalue.at(1));
                        continue;
                    }
                }
            }

            if (!value.sound_effects.empty()) {
                for (const auto &values : value.sound_effects) {
                    try {
                        auto spvalue = KMCSplit(values.ref, ',');
                        
                        if (spvalue.size() == 2) {
                            try {

                                auto* sd = (RE::BGSSoundDescriptorForm*)RE::TESDataHandler::GetSingleton()->LookupForm(
                                    std::stoll(spvalue.at(0), NULL, 16), spvalue.at(1));
                                if (sd == nullptr) {
                                    ERROR(
                                        "ERROR Possibly wrong definition of SoundDescriptorFormSE. key: {} FORM ID: {} {}",
                                        track_id, spvalue.at(0), spvalue.at(1));
                                    continue;
                                }
                                RE::Actor* SEPoint = player;
                                if (values.emit_from != PLAY_SE_PLAYER_POS) {
                                    if (follower == nullptr) {
                                        ERROR(
                                            "ERROR LOADING SoundEffect.json. Error at nested node. not found "
                                            "follower {}",
                                            track_id);
                                        continue;
                                    } else {
                                        SEPoint = follower;
                                    }
                                }

                                std::string record = std::to_string(values.timing);
                                if (target_sound_effect->contains(key)) {
                                    const auto& recordmap = (*target_sound_effect).at(key);
                                    if (recordmap.contains(record)) {
                                        LOG("SoundDiscription SE Sound effects cannot be played at the same "
                                            "time.Please use different values. = key:{} FORM ID:{} {} record:{}",
                                            track_id, spvalue.at(0), spvalue.at(1), record);
                                        continue;
                                    }
                                }
                                (*target_sound_effect)[key][record] = KMCSECond(sd, SEPoint);

                                LOG("SoundDiscription SE Loaded = key:{} FORM ID:{} {} record:{}", track_id,
                                    spvalue.at(0), spvalue.at(1),
                                    record);
                            } catch (...) {
                                ERROR("ERROR LOADING SoundDescriptorSEFormId.json. Error at nested node. {}", track_id);
                            }
                        }
                    } catch (...) {
                        ERROR("ERROR LOADING SoundDiscription.json. The number of elements in the value is wrong.");
                        // return;
                    }
                }
            }
        }
    }

    void KMCSound::Init() {
        // sound desc mapping

        const auto player_addon_set = KMCDisplayAddon::GetSingleton()->GetActorAddons((int)KMCDisplayType::PLAYER);
        const auto player = KMCCT::KMCConfig::GetSingleton()->GetPlayer();
        InitLoop(&sound_descriptor_map, &sound_descriptor_se_map, player_addon_set, player, nullptr);

        auto* followers = KMCCT::KMCConfig::GetSingleton()->GetFollowers();
        for (const auto& f : *followers) {

            std::vector<std::pair<int, RE::BGSSoundDescriptorForm*>> sd;
            std::map<int, std::map<std::string, KMCSECond>> sd_se_map;
            std::vector<std::pair<std::string, size_t>> si;

            const auto follower_addon_set = KMCDisplayAddon::GetSingleton()->GetActorAddons(f.index + 1);

            InitLoop(&sd, &sd_se_map, follower_addon_set, player, f.follower);

            FSoundDescriptiorMap.push_back(std::make_pair(f.index, KMCFSoundDescription(sd, sd_se_map)));        
        }


        // Profile SE
        {
            auto pse = KMCCT::KMCConfig::GetSingleton()->getIProfileSoundEffect();
            int index = 0;
            for (auto& [key, value] : *pse) {
                auto spvalue = KMCSplit(value, ',');
                std::string formId = "";
                std::string pluginn = "";

                try {
                    formId = spvalue.at(0);
                    pluginn = spvalue.at(1);
                    auto* sd = (RE::BGSSoundDescriptorForm*)RE::TESDataHandler::GetSingleton()->LookupForm(
                        std::stoll(formId, NULL, 16), pluginn);
                    if (sd != nullptr) {
                        ProfileSE.insert(std::make_pair(index, sd));
                    }
                } catch (...) {
                    pluginn = SD_PLUGIN_NAME;
                }


                ++index;
            }
        }

        AudioManager = RE::BSAudioManager::GetSingleton();
    }

    void KMCSound::PlayEx(int trackid, float volume, RE::TESObjectREFR* target, int frand) {
        if (frand == -1) {
            Play(trackid, volume, target);
        } else {
            Play(trackid, volume, target, frand);
        }
    }

    bool KMCSound::GetFirstSEIndexEx(int trackid, int frand, std::string* fstrecord) {
        if (frand == -1) {
            *fstrecord = GetFirstSEIndex(trackid);
        } else {
            *fstrecord = GetFirstSEIndex(trackid, frand);
        }

        if (*fstrecord == "") {
            return false;
        } else {
            return true;
        }
    }

    bool KMCSound::PlaySEEx(int trackid, int frand, std::string* record, float volume) {
        LOG("KMCSound::PlaySEEx");

        std::string findid = trackid + "," + *record;
        if (frand == -1) {
            *record = PlaySE(trackid, *record, findid, volume);
        } else {
            *record = PlaySE(trackid, frand, *record, findid, volume);
        }

        if (*record == "") {
            return false;
        } else {
            return true;
        }
    }

    bool KMCSound::IsPlayableSoundEx(int trackid, int frand) {
        if (frand == -1) {
            return IsPlayableSound(trackid);
        } else {
            return IsPlayableSound(trackid, frand);
        }
    }

    void KMCSound::PlayProfileSE(KMCProfileSEType trackid, float volume, RE::TESObjectREFR* target) {
        int tid = (int)trackid;
        if (ProfileSE.contains(tid)) {
            auto se = ProfileSE.at(tid);
            RE::BSSoundHandle handle;
            AudioManager->BuildSoundDataFromDescriptor(handle, se);
            handle.SetVolume(volume);
            handle.SetObjectToFollow(target->Get3D());
            handle.Play();
        }
    }

    void KMCSound::Play(int trackid, float volume, RE::TESObjectREFR* target) {
        auto it = std::find_if(sound_descriptor_map.begin(), sound_descriptor_map.end(),
                                [trackid](const auto& p) { return p.first == trackid; });
        if (it != sound_descriptor_map.end()) {
            RE::BSSoundHandle handle;
            AudioManager->BuildSoundDataFromDescriptor(handle, (it->second)->soundDescriptor);
            handle.SetVolume(volume);
            handle.SetObjectToFollow(target->Get3D());
            handle.Play();
        } else {
            LOG("KMCSound Player::Play the pair was not found trackid = {}", trackid);
        }
    }

    void KMCSound::Play(int trackid, float volume, RE::TESObjectREFR* target, int frand) {

        auto it = std::find_if(FSoundDescriptiorMap.begin(), FSoundDescriptiorMap.end(),
                                [frand](const auto& p) { return p.first == frand; }); 
        if (it != FSoundDescriptiorMap.end()) {
            auto fsd = it->second.sd_map;
            auto it2 =
                std::find_if(fsd.begin(), fsd.end(), [trackid](const auto& p) { return p.first == trackid; });
            if (it2 != fsd.end()) {
                RE::BSSoundHandle handle;
                if ((it2->second)->soundDescriptor == nullptr) {
                    LOG("KMCSound::Play follower {} track id {}", frand, trackid);
                }
                AudioManager->BuildSoundDataFromDescriptor(handle, (it2->second)->soundDescriptor);
                handle.SetVolume(volume);
                handle.SetObjectToFollow(target->Get3D());
                handle.Play();
            } else {
                LOG("KMCSound Follower::Play the pair was not found trackid = {}", trackid);
            }
        }
    }

    std::string KMCSound::GetFirstSEIndex(int trackid, int frand) {
        auto itf = std::find_if(FSoundDescriptiorMap.begin(), FSoundDescriptiorMap.end(),
                                [frand](const auto& p) { return p.first == frand; });
        if (itf != FSoundDescriptiorMap.end()) {
            auto semap = itf->second.sd_se_map;

            if (semap.contains(trackid)) {
                auto recordmap = semap[trackid];
                return recordmap.begin()->first;
            }

            //auto it = std::find_if(si.begin(), si.end(),
            //                       [trackid](const auto& p) { return KMCSplit(p.first, ',').at(0) == trackid; });
            //if (it != si.end()) {
            //    return KMCSplit(it->first, ',').at(1);
            //}
        }

        return "";
    }

    std::string KMCSound::GetFirstSEIndex(int trackid) {

        if (sound_descriptor_se_map.contains(trackid)) {
            auto recordmap = sound_descriptor_se_map[trackid];
            return recordmap.begin()->first;
        }
        //auto it = std::find_if(SEIndex.begin(), SEIndex.end(), [trackid](const auto& p) { 
        //    return KMCSplit(p.first, ',').at(0) == trackid;
        //});
        //if (it != SEIndex.end()) {
        //    return KMCSplit(it->first, ',').at(1);
        //}

        return "";
    }

    std::string KMCSound::PlaySE(int trackid, int frand, std::string record, std::string findid,
                                   float volume) {

        auto itf = std::find_if(FSoundDescriptiorMap.begin(), FSoundDescriptiorMap.end(),
                        [frand](const auto& p) { return p.first == frand; });
        if (itf != FSoundDescriptiorMap.end()) {
            auto semap = itf->second.sd_se_map;
            if (semap.contains(trackid)) {
                auto recordmap = semap[trackid];

                if (recordmap.contains(record)) {
                    auto v = recordmap[record];
                    LOG("PlaySE Follower : trackid {} record {} pos {}", trackid, record,
                        v.SEPoint->GetName());
                    RE::BSSoundHandle handle;
                    AudioManager->BuildSoundDataFromDescriptor(handle, v.sd);
                    handle.SetVolume(volume);
                    handle.SetObjectToFollow(v.SEPoint->Get3D());
                    handle.Play();

                    // next record
                    bool is_next = false;
                    for (auto& [record_key, record_value] : recordmap) {
                        if (is_next) {
                            return record_key;
                        }

                        if (record_key == record) {
                            is_next = true;
                        }
                    }
                }
            }

            // auto si = itf->second.SEIndex;
            //auto it = std::find_if(si.begin(), si.end(), [findid](const auto& p) { return p.first == findid; });
            //if (it != si.end()) {
            //    size_t offset = it->second;
            //    auto sdsem = itf->second.SDSEMap;
            //    LOG("PlaySE Follower : Found findid {} offset {}", findid, offset);
            //    auto it_sdsem = sdsem.begin() + offset;
            //    for (; it_sdsem != sdsem.end(); ++it_sdsem) {
            //        std::string k = it_sdsem->first;
            //        auto v = it_sdsem->second;
            //        auto sp = KMCSplit(k, ',');
            //        std::string tid = sp.at(0);
            //        std::string rid = sp.at(1);

            //        LOG("PlaySE Follower : tid {} rid {} trackid{} record{}", tid, rid, trackid, record);

            //        if (tid == trackid && rid == record) {
            //            LOG("PlaySE Follower : trackid {} record {} play_now {} pos {}", trackid, record, k,
            //                v.SEPoint->GetName());
            //            RE::BSSoundHandle handle;
            //            AudioManager->BuildSoundDataFromDescriptor(handle, v.sd);
            //            handle.SetVolume(volume);
            //            handle.SetObjectToFollow(v.SEPoint->Get3D());
            //            handle.Play();
            //        } else if (tid == trackid && rid != record) {
            //            LOG("PlaySE Follower : Next Record {}", rid);
            //            return rid;
            //        } else {
            //            break;
            //        }
            //    }
            //}
        }

        return "";
    }

    std::string KMCSound::PlaySE(int trackid, std::string record, std::string findid, float volume) {    

        if (sound_descriptor_se_map.contains(trackid)) {
            auto recordmap = sound_descriptor_se_map[trackid];

            if (recordmap.contains(record)) {
                auto v = recordmap[record];
                LOG("PlaySE Player : trackid{} record{}", trackid, record);
                RE::BSSoundHandle handle;
                AudioManager->BuildSoundDataFromDescriptor(handle, v.sd);
                handle.SetVolume(volume);
                handle.SetObjectToFollow(v.SEPoint->Get3D());
                handle.Play();

                // next record
                bool is_next = false;
                for (auto& [record_key, record_value] : recordmap) {
                    if (is_next) {
                        return record_key;
                    }

                    if (record_key == record) {
                        is_next = true;
                    }
                }
            }
        }
        //auto it = std::find_if(SEIndex.begin(), SEIndex.end(), [findid](const auto& p) { return p.first == findid; });
        //if (it != SEIndex.end()) {
        //    size_t offset = it->second;

        //    LOG("PlaySE Player : Found findid {} offset {}", findid, offset);
        //    auto it_sdsem = SoundDescriptorSEMap.begin() + offset;
        //    for (; it_sdsem != SoundDescriptorSEMap.end(); ++it_sdsem) {
        //        std::string k = it_sdsem->first;
        //        auto v = it_sdsem->second;
        //        auto sp = KMCSplit(k, ',');
        //        std::string tid = sp.at(0);
        //        std::string rid = sp.at(1);

        //        LOG("PlaySE Player : tid {} rid {} trackid{} record{}", tid, rid, trackid, record);

        //        if (tid == trackid && rid == record) {
        //            LOG("PlaySE Player : trackid {} record {} play_now {} pos {}", trackid, record, k,
        //                v.SEPoint->GetName());
        //            RE::BSSoundHandle handle;
        //            AudioManager->BuildSoundDataFromDescriptor(handle, v.sd);
        //            handle.SetVolume(volume);
        //            handle.SetObjectToFollow(v.SEPoint->Get3D());
        //            handle.Play();
        //        } else if (tid == trackid && rid != record) {
        //            LOG("PlaySE Player : Next Record {}", rid);
        //            return rid;
        //        } else {
        //            break;
        //        }
        //    }
        //}
        return "";
    }

    bool KMCSound::IsPlayableSound(int trackid, int frand) {
        auto it = std::find_if(FSoundDescriptiorMap.begin(), FSoundDescriptiorMap.end(),
                               [frand](const auto& p) { return p.first == frand; });
        if (it != FSoundDescriptiorMap.end()) {
            auto fsd = it->second.sd_map;
            auto it2 = std::find_if(fsd.begin(), fsd.end(), [trackid](const auto& p) { return p.first == trackid; });
            if (it2 != fsd.end()) {
                return true;
            }
        }

        return false;
    }
    bool KMCSound::IsPlayableSound(int trackid) {
        auto it = std::find_if(sound_descriptor_map.begin(), sound_descriptor_map.end(),
                               [trackid](const auto& p) { return p.first == trackid; });
        if (it != sound_descriptor_map.end()) {
            return true;
        }

        return false;
    }
}