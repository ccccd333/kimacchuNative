#include "KMCSound.h"
#include "KMCConfig.h"
#include <IWWConfig.h>

SINGLETONBODY(KMCCT::KMCSound)

namespace KMCCT {
    using namespace boost::property_tree;

    void InitLoop(std::vector<std::pair<std::string, RE::BGSSoundDescriptorForm*>> *target,
                  std::vector<std::pair<std::string, std::string>>* sdfc,
                  std::map<std::string, std::map<std::string, KMCSECond>>* targetSE,
                  std::vector<std::pair<std::string, std::map<std::string, std::string>>>* sdse, RE::Actor* player,
                  RE::Actor* follower) {
        {
            for (auto [key, value] : *sdfc) {
                try {
                    std::string trackid = key;
                    auto spvalue = KMCSplit(value, ',');
                    std::string pluginn = SD_PLUGIN_NAME;

                    try {
                        pluginn = spvalue.at(1);
                    } catch (...) {
                        pluginn = SD_PLUGIN_NAME;
                    }

                    auto* sd = (RE::BGSSoundDescriptorForm*)RE::TESDataHandler::GetSingleton()->LookupForm(
                        std::stoll(spvalue.at(0), NULL, 16), pluginn);
                    if (sd == nullptr) {
                        ERROR("ERROR Possibly wrong definition of SoundDescriptorForm. key: {} FORM ID: {} {}", key,
                              value, pluginn);
                        continue;
                    }

                    target->push_back(std::make_pair(trackid, sd));
                    LOG("SoundDiscription Loaded = key:{} FORM ID:{}", trackid, value);
                } catch (...) {
                    ERROR("ERROR LOADING SoundDiscription.json. The number of elements in the value is wrong.");
                    // return;
                }
            }
        }

        {
            for (auto [key2, value2] : *sdse) {
                try {
                    std::string trackid = key2;

                    for (auto [key3, value3] : value2) {
                        try {
                            auto spvalue = KMCSplit(value3, ',');
                            std::string pluginn = SD_PLUGIN_NAME;
                            try {
                                pluginn = spvalue.at(1);
                            } catch (...) {
                            }

                            auto* sd = (RE::BGSSoundDescriptorForm*)RE::TESDataHandler::GetSingleton()->LookupForm(
                                std::stoll(spvalue.at(0), NULL, 16), pluginn);
                            if (sd == nullptr) {
                                ERROR("ERROR Possibly wrong definition of SoundDescriptorFormSE. key: {} FORM ID: {}",
                                      key2, value3);
                                continue;
                            }
                            RE::Actor* SEPoint = player;
                            try {
                                if (std::stoi(spvalue.at(3)) != PLAY_SE_PLAYER_POS) {
                                    if (follower == nullptr) {
                                        ERROR(
                                            "ERROR LOADING SoundEffect.json. Error at nested node. not found follower {}",
                                            key3);
                                        continue;
                                    } else {
                                        SEPoint = follower;
                                    }
                                }
                            } catch (...) {
                            }
                            std::string record = spvalue.at(2);
                            if (targetSE->contains(trackid)) {
                                auto recordmap = (*targetSE)[trackid];
                                if (recordmap.contains(record)) {
                                    LOG("SoundDiscription SE Sound effects cannot be played at the same time.Please use different values. = key:{} FORM ID:{} record:{}",
                                        trackid, value3,
                                        record);
                                    continue;
                                }
                            }
                            (*targetSE)[trackid][record] = KMCSECond(sd, SEPoint);

                            LOG("SoundDiscription SE Loaded = key:{} FORM ID:{} record:{}", trackid, value3, record);
                        } catch (...) {
                            ERROR("ERROR LOADING SoundDescriptorSEFormId.json. Error at nested node. {}", key3);
                        }
                    }
                } catch (...) {
                    ERROR("ERROR LOADING SoundDescriptorSEFormId.json. The number of elements in the value is wrong.");
                    // return;
                }
            }
        }
    }
    
    //void BuildIndex(std::vector<std::pair<std::string, KMCSECond>>* SE,
    //                std::vector<std::pair<std::string, size_t>>* target) {
    //    std::vector<std::pair<std::string, KMCSECond>> copy(SE->size()); 
    //    std::copy(SE->begin(), SE->end(), copy.begin());

    //    size_t offset = 0;
    //    size_t origin = 0;
    //    
    //    auto i = SE->begin();
    //    for (; i != SE->end();) {
    //        std::vector<std::pair<std::string, size_t>> vec;
    //        auto sp = KMCSplit(i->first, ',');
    //        std::string tid = sp[0];
    //        std::string rid = sp[1];
    //        std::string irid = "";

    //        auto it = copy.begin() + offset;
    //        for (; it != copy.end(); ++it) {
    //            auto isp = KMCSplit(it->first, ',');
    //            std::string iitid = isp[0];
    //            std::string iirid = isp[1];
    //            if (tid == iitid && rid == iirid) {
    //                irid = iirid;
    //                ++offset;
    //            } else {
    //                vec.push_back(std::make_pair(rid, origin));
    //                LOG("BuildIndex k1 {} v1 {} k2 {} v2 {} of {} or {}", tid, rid, iitid, iirid, offset, origin);
    //                break;
    //            }
    //        }

    //        if (it == copy.end()) {
    //            LOG("BuildIndex k1 {} v1 {} of {} or {}", tid, rid, offset, origin);
    //            target->push_back(std::make_pair(tid + "," + irid, origin));
    //            break;
    //        } else {
    //            target->push_back(std::make_pair(tid + "," + irid, origin));
    //            i = i + offset - origin;
    //            origin = offset;
    //        }
    //    }
    //}
    
    //bool compare(std::pair<std::string, KMCSECond>& p1, std::pair<std::string, KMCSECond>& p2) {
    //    return p1.first < p2.first;
    //}
    void KMCSound::Init() {
        // sound desc mapping
        
        auto soundDescriptorFormId = KMCCT::KMCConfig::GetSingleton()->getISoundDescriptorFormIdConfigs();
        auto soundDescriptorSEFormId = KMCCT::KMCConfig::GetSingleton()->getISoundDescriptorSEFormIdConfigs();
        InitLoop(&SoundDescriptorMap, soundDescriptorFormId, &SoundDescriptorSEMap, soundDescriptorSEFormId,
                 KMCCT::KMCConfig::GetSingleton()->getPlayer(), nullptr);
        //std::sort(SoundDescriptorSEMap.begin(), SoundDescriptorSEMap.end(), compare);
        //BuildIndex(&SoundDescriptorSEMap, &SEIndex);

        auto* followers = KMCCT::KMCConfig::GetSingleton()->getFollowers();
        for (int i = 0; i < followers->size(); i++) {
            KMCFollower f = (*followers)[i];

            std::vector<std::pair<std::string, RE::BGSSoundDescriptorForm*>> sd;
            std::map<std::string, std::map<std::string, KMCSECond>> SDSEMap;
            std::vector<std::pair<std::string, size_t>> si;
            InitLoop(&sd, &(f.ISoundDescriptorFormIdConfigs), &SDSEMap, &(f.ISoundDescriptorSEFormIdConfigs),
                     KMCCT::KMCConfig::GetSingleton()->getPlayer(), f.follower);
            //std::sort(SDSEMap.begin(), SDSEMap.end(), compare);
            //BuildIndex(&SDSEMap, &si);

            FSoundDescriptiorMap.push_back(std::make_pair(i, KMCFSoundDescription(sd, SDSEMap, si)));
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

    void KMCSound::PlayEx(std::string trackid, float volume, RE::TESObjectREFR* target, int frand) {
        if (frand == -1) {
            Play(trackid, volume, target);
        } else {
            Play(trackid, volume, target, frand);
        }
    }

    bool KMCSound::GetFirstSEIndexEx(std::string trackid, int frand, std::string* fstrecord) {
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

    bool KMCSound::PlaySEEx(std::string trackid, int frand, std::string* record, float volume) {
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

    bool KMCSound::IsPlayableSoundEx(std::string trackid, int frand) {
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

    void KMCSound::Play(std::string trackid, float volume, RE::TESObjectREFR* target) {
        auto it = std::find_if(SoundDescriptorMap.begin(), SoundDescriptorMap.end(),
                                [trackid](const auto& p) { return p.first == trackid; });
        if (it != SoundDescriptorMap.end()) {
            RE::BSSoundHandle handle;
            AudioManager->BuildSoundDataFromDescriptor(handle, (it->second)->soundDescriptor);
            handle.SetVolume(volume);
            handle.SetObjectToFollow(target->Get3D());
            handle.Play();
        } else {
            LOG("KMCSound Player::Play the pair was not found trackid = {}", trackid);
        }
    }

    void KMCSound::Play(std::string trackid, float volume, RE::TESObjectREFR* target, int frand) {

        auto it = std::find_if(FSoundDescriptiorMap.begin(), FSoundDescriptiorMap.end(),
                                [frand](const auto& p) { return p.first == frand; }); 
        if (it != FSoundDescriptiorMap.end()) {
            auto fsd = it->second.SoundDescriptorMap;
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

    std::string KMCSound::GetFirstSEIndex(std::string trackid, int frand) {
        auto itf = std::find_if(FSoundDescriptiorMap.begin(), FSoundDescriptiorMap.end(),
                                [frand](const auto& p) { return p.first == frand; });
        if (itf != FSoundDescriptiorMap.end()) {
            auto semap = itf->second.SDSEMap;

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

    std::string KMCSound::GetFirstSEIndex(std::string trackid) {

        if (SoundDescriptorSEMap.contains(trackid)) {
            auto recordmap = SoundDescriptorSEMap[trackid];
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

    std::string KMCSound::PlaySE(std::string trackid, int frand, std::string record, std::string findid,
                                   float volume) {

        auto itf = std::find_if(FSoundDescriptiorMap.begin(), FSoundDescriptiorMap.end(),
                        [frand](const auto& p) { return p.first == frand; });
        if (itf != FSoundDescriptiorMap.end()) {
            auto semap = itf->second.SDSEMap;
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

    std::string KMCSound::PlaySE(std::string trackid, std::string record, std::string findid, float volume) {    

        if (SoundDescriptorSEMap.contains(trackid)) {
            auto recordmap = SoundDescriptorSEMap[trackid];

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

    bool KMCSound::IsPlayableSound(std::string trackid, int frand) {
        auto it = std::find_if(FSoundDescriptiorMap.begin(), FSoundDescriptiorMap.end(),
                               [frand](const auto& p) { return p.first == frand; });
        if (it != FSoundDescriptiorMap.end()) {
            auto fsd = it->second.SoundDescriptorMap;
            auto it2 = std::find_if(fsd.begin(), fsd.end(), [trackid](const auto& p) { return p.first == trackid; });
            if (it2 != fsd.end()) {
                return true;
            }
        }

        return false;
    }
    bool KMCSound::IsPlayableSound(std::string trackid) {
        auto it = std::find_if(SoundDescriptorMap.begin(), SoundDescriptorMap.end(),
                               [trackid](const auto& p) { return p.first == trackid; });
        if (it != SoundDescriptorMap.end()) {
            return true;
        }

        return false;
    }
}