#include "KMCCutin.h"

#include "KMCConfig.h"
#include "KMCContextManager.h"
#include "KMCDisplayWordAndTexture.h"
#include "KMCEventThread.h"
#include "KMCExpression.h"
#include "KMCOAR.h"
#include "KMCPrismaUIBridge.h"
#include "KMCSound.h"
#include "KMCStateManager.h"
#include "KMCWaitTask.h"

SINGLETONBODY(KMCCT::KMCCutin)

namespace KMCCT {
    std::map<int, KMCCutinOrder> AnimFtoAnimP{
        {0, KMCCutinOrder(KMCCT::KMCOnCutinStartReady, 1)},  {1, KMCCutinOrder(KMCCT::KMCExpFuncStart, 1)},
        {2, KMCCutinOrder(KMCCT::KMCOARFuncStart, 1)},       {13, KMCCutinOrder(KMCCT::KMCPlayAnim, 1)},
        {16, KMCCutinOrder(KMCCT::KMCOnCutinEnd, 1)},

        {17, KMCCutinOrder(KMCCT::KMCOnCutinStartReady, 0)}, {18, KMCCutinOrder(KMCCT::KMCExpFuncStart, 0)},
        {19, KMCCutinOrder(KMCCT::KMCOARFuncStart, 0)},      {23, KMCCutinOrder(KMCCT::KMCPlayAnim, 0)},
        {26, KMCCutinOrder(KMCCT::KMCOnCutinEnd, 0)},

        {30, KMCCutinOrder(KMCCT::KMCBatchPreloadGroups, 0)}};
    std::map<int, KMCCutinOrder> AnimPtoAnimF{
        {0, KMCCutinOrder(KMCCT::KMCOnCutinStartReady, 0)},  {1, KMCCutinOrder(KMCCT::KMCExpFuncStart, 0)},
        {2, KMCCutinOrder(KMCCT::KMCOARFuncStart, 0)},       {13, KMCCutinOrder(KMCCT::KMCPlayAnim, 0)},
        {16, KMCCutinOrder(KMCCT::KMCOnCutinEnd, 0)},

        {17, KMCCutinOrder(KMCCT::KMCOnCutinStartReady, 1)}, {18, KMCCutinOrder(KMCCT::KMCExpFuncStart, 1)},
        {19, KMCCutinOrder(KMCCT::KMCOARFuncStart, 1)},      {23, KMCCutinOrder(KMCCT::KMCPlayAnim, 1)},
        {26, KMCCutinOrder(KMCCT::KMCOnCutinEnd, 1)},        {30, KMCCutinOrder(KMCCT::KMCBatchPreloadGroups, 0)}};
    std::map<int, KMCCutinOrder> AnimPlayerOnly{{0, KMCCutinOrder(KMCCT::KMCOnCutinStartReady, 0)},
                                                {1, KMCCutinOrder(KMCCT::KMCExpFuncStart, 0)},
                                                {2, KMCCutinOrder(KMCCT::KMCOARFuncStart, 0)},
                                                {13, KMCCutinOrder(KMCCT::KMCPlayAnim, 0)},
                                                {16, KMCCutinOrder(KMCCT::KMCOnCutinEnd, 0)}};

    std::string ct_aaaakmcroot = "";
    KMCInterruptPushCutInData aaaakmcInterruptData;
    float aaaakmcvolum = 0.5f;
    long long aaaakmcAnimtime = 4;
    long long aaaakmctime = 5;
    long long aaaakmcCycle = 10;

    std::string aaaaPlayerTextX = "0";
    std::string aaaaPlayerTextY = "0";
    std::string aaaaPlayerWidgetX = "0";
    std::string aaaaPlayerWidgetY = "0";

    std::string aaaaPlayerNTextX = "0";
    std::string aaaaPlayerNTextY = "0";
    std::string aaaaPlayerNWidgetX = "0";
    std::string aaaaPlayerNWidgetY = "0";

    std::string aaaaFollowerTextX = "0";
    std::string aaaaFollowerTextY = "0";
    std::string aaaaFollowerWidgetX = "0";
    std::string aaaaFollowerWidgetY = "0";

    std::string aaaaFollowerNTextX = "0";
    std::string aaaaFollowerNTextY = "0";
    std::string aaaaFollowerNWidgetX = "0";
    std::string aaaaFollowerNWidgetY = "0";

    std::string KMC_DEFAULT_FONT = "$EverywhereFont";
    int KMC_DEFAULT_FONT_SIZE = 24;
    int KMC_DEFAULT_NAME_POSX = 640;
    int KMC_DEFAULT_NAME_POSY = 320;
    int KMC_DEFAULT_WIDGET_POS_X = 640;
    int KMC_DEFAULT_WIDGET_POS_Y = 460;
    int KMC_DEFAULT_TEXT_POS_X = 640;
    int KMC_DEFAULT_TEXT_POS_Y = 535;

    void KMCCutin::InitCutin() {
        animnow = false;

        aaaakmcvolum = 1.0f;
        aaaakmcAnimtime = 5.0f;
        aaaakmctime = 5.0f;
        aaaakmcCycle = 10.0f;
    }

    void KMCCutin::InterruptCutInEventManager(KMCInterruptPushCutInData data) {
        static long long event_cool_time =
            KMCFindVector(KMCCT::KMCConfig::GetSingleton()->GetKMCSetting(), KMCCT::STATE_MANAGER_CONFIG_KEY,
                          KMCCT::INTERRUPT_EVENT_COOL_TIME) *
            KMCCT::TIME_SCALE_MS;
        auto *thread = KMCCT::KMCEventThread::GetSingleton();
        if (!thread->GetForceEndAnim() && !thread->IsShuttingDown() && thread->GetInitEndFlag() &&
            !KMCCT::KMCCutin::GetSingleton()->GetAnimNow() && !KMCCT::KMCWaitTask::GetSingleton()->GetWaitFlag()) {
            time_point<Clock> interrupt_time = Clock::now();
            milliseconds diff = duration_cast<milliseconds>(interrupt_time - event_start);
            long long dur = diff.count();
            if (dur < event_cool_time) {
                KMC_LOG("InterruptCutInEventManager cool time {} {}", dur, event_cool_time);
                return;
            }
            event_start = Clock::now();
            aaaakmcInterruptData = data;
            // LOG("InterruptCutInEventManager interrupt");

            switch (aaaakmcInterruptData.interrupt_type) {
                case KMCInterruptType::add_item:
                    KMCCT::wrap_InterruptCutInEventManager(InterruptEventAddItem);
                    break;
                case KMCInterruptType::on_hit:
                    KMCCT::wrap_InterruptCutInEventManager(InterruptEventOnHit);
                    break;
            }
        }
    }

    int KMCCutin::CondCutIn(KMCCutinValues val) {
        auto node = KMCCT::KMCCutinCondition::GetSingleton()->GetTaskResult();

        if (node) {
            // guard
            {
                std::lock_guard<std::mutex> lock(animnow_mtx);
                if (animnow) return -1;
                animnow = true;
            }

            node->Tuning(val);

            KMCCT::KMCCutinCondition::GetSingleton()->Completed(node);

            // qu exp
            if (KMCCT::KMCExpression::GetSingleton()->OnStandby() < 0) {
                return 99;
            }

            int rslt = CutIn(val);

            // pop exp
            KMCCT::KMCExpression::GetSingleton()->OnFinished();

            return rslt;
        }

        return 99;
    }
    int KMCCutin::IterCutIn(KMCCutinValues val) {
        // guard
        {
            std::lock_guard<std::mutex> lock(animnow_mtx);
            if (animnow) return -1;
            animnow = true;
        }

        return CutIn(val);
    }

    // return = -1 : 他のカットイン要求を処理中
    // return = -2 : StateManager(プレイヤーの状態管理)により拒否された(プレイヤーが話し中など)
    // return = 2 : rand生成したが対象がいなかった
    // return = 0 : 正常終了
    int KMCCutin::CutIn(KMCCutinValues val) {
        // init
        std::string aaaakmctype = val.aaaakmctype;
        float c_volum = val.aaaakmcvolum;
        float c_oar_time = val.aaaakmcoar;
        float c_exp_time = val.aaaakmcexp;

        if (aaaakmctype == "") {
            aaaakmctype = KMCContextManager::GetSingleton()->GetPlayerState();

            if (aaaakmctype == "") {
                std::lock_guard<std::mutex> lock(animnow_mtx);
                animnow = false;
                return -2;
            }
        }
        KMC_LOG("CutIn type {}", aaaakmctype);

        int exp_rand = -1;
        std::string exp_type = val.aaaakmcExptype;
        KMC_LOG("EXP type ===> {}, CUTIN type ===> {}", exp_type, aaaakmctype);
        if (val.overri_fc_exp) {
            KMC_LOG("[EXP type] ===> {}", exp_type);
            if (aaaakmctype != exp_type) {
                exp_rand = KMCCT::KMCExpression::GetSingleton()->GetCutInID(exp_type);
                if (exp_rand == 0) {
                    KMC_LOG("EXP type ===> not found", exp_type, aaaakmctype);
                    exp_rand = -1;
                }
            }
        }

        bool can_follower_cutin = true;
        RE::ActorHandle follower;
        std::unordered_map<int, std::string> *playback_priority = nullptr;
        KMCCompsFlag pcf;
        KMCCompsFlag fcf;
        std::string precord = "";
        std::string frecord = "";

        // player
        int rand = GetCutInID(aaaakmctype);
        int next_rand = PeekNextCutInID(aaaakmctype);
        if (rand < 0 || next_rand < 0) {
            std::lock_guard<std::mutex> lock(animnow_mtx);
            animnow = false;
            return 2;
        }
        std::string srand = std::to_string(rand);

        const auto &player_cutin_entry =
            KMCDisplayWordAndTexture::GetSingleton()->GetEntriesDataMap((int)KMCDisplayType::PLAYER).at(rand);

        pcf.Sound = KMCCT::KMCSound::GetSingleton()->IsPlayableSoundEx(rand, -1);

        pcf.SE = KMCCT::KMCSound::GetSingleton()->GetFirstSEIndexEx(rand, -1, &precord);
        auto *thread = KMCCT::KMCEventThread::GetSingleton();
        if (thread->GetForceEndAnim() || thread->IsShuttingDown()) {
            return -99;
        }

        // follower
        int frand = -1;
        int f_next_rand_id = -1;
        auto f = FollowerEffectiveDistance(KMCCT::KMCConfig::GetSingleton()->GetPlayer(),
                                           KMCCT::KMCConfig::GetSingleton()->GetFollowers(), followerDetectRange, rand);

        CutinEntry follower_cutin_entry;
        if (f.size() != 0) {
            frand = f.begin()->index;
            int target_index = 0;
            KMC_LOG("fsize {}", f.size());
            if (f.size() > 1) {
                std::random_device rnd;
                std::mt19937 mt(rnd());
                std::uniform_int_distribution<> randhl(0, (int)(f.size()) - 1);
                target_index = randhl(mt);
                frand = f[target_index].index;
            }

            const auto &fentry = KMCDisplayWordAndTexture::GetSingleton()->GetEntriesDataMap(frand + 1);
            if (fentry.contains(rand)) {
                follower = f[target_index].follower_handle;
                if (!follower.get()) {
                    can_follower_cutin = false;
                }
                playback_priority = &(f[target_index].playback_priority);
                fcf.Sound = KMCCT::KMCSound::GetSingleton()->IsPlayableSoundEx(rand, frand);

                fcf.SE = KMCCT::KMCSound::GetSingleton()->GetFirstSEIndexEx(rand, frand, &frecord);

                follower_cutin_entry = fentry.at(rand);
            } else {
                can_follower_cutin = false;
            }

            if (fcf.IsAllDisable()) {
                can_follower_cutin = false;
            }

            if (fentry.contains(next_rand)) {
                f_next_rand_id = next_rand;
            }
        } else {
            can_follower_cutin = false;
        }

        if (can_follower_cutin) {
            auto *player = KMCCT::KMCConfig::GetSingleton()->GetPlayer();
            KMCAnimST st = KMCAnimST();
            st.t = player_cutin_entry;
            st.ft = follower_cutin_entry;
            st.time = player_cutin_entry.display_time * KMCCT::TIME_SCALE_MS;
            st.ftime = follower_cutin_entry.display_time * KMCCT::TIME_SCALE_MS;
            st.rand = rand;
            st.frand = frand;
            st.p_next_rand = next_rand;
            st.f_netx_rand = f_next_rand_id;
            st.volum = c_volum;
            st.playback_priority = playback_priority;
            st.speakerp = player;
            st.speakerf_handle = follower;
            st.pcf = pcf;
            st.fcf = fcf;
            st.precord = precord;
            st.frecord = frecord;
            st.oar_time = c_oar_time;
            st.exp_time = c_exp_time;
            st.overri_oar_time = val.overri_oar_time;
            st.overri_exp_time = val.overri_exp_time;
            st.exp_rand = exp_rand;
            st.player_name = player->GetName();
            st.follower_name = follower.get()->GetName();

            PlayDuoCutin(st);
        } else {
            auto *player = KMCCT::KMCConfig::GetSingleton()->GetPlayer();
            KMCAnimST st = KMCAnimST();
            st.time = player_cutin_entry.display_time * KMCCT::TIME_SCALE_MS;
            st.rand = rand;
            st.frand = frand;
            st.p_next_rand = next_rand;
            st.volum = c_volum;
            st.speakerp = player;
            st.pcf = pcf;
            st.precord = precord;
            st.oar_time = c_oar_time;
            st.exp_time = c_exp_time;
            st.overri_oar_time = val.overri_oar_time;
            st.overri_exp_time = val.overri_exp_time;
            st.exp_rand = exp_rand;
            st.player_name = player->GetName();
            PlaySoloCutin(st);
        }

        {
            std::lock_guard<std::mutex> lock(animnow_mtx);
            animnow = false;
        }

        return 0;
    }

    bool KMCCutin::GetAnimNow() {
        {
            std::lock_guard<std::mutex> lock(animnow_mtx);
            return animnow;
        }
    }
    void KMCCutin::SetAnimNow(bool set) {
        {
            std::lock_guard<std::mutex> lock(animnow_mtx);
            animnow = set;
        }
    }

    bool KMCCutin::GetCutinStartReady() { return cutin_start_ready.load(); }
    void KMCCutin::SetCutinStartReady(bool set) { cutin_start_ready.store(set); }

    bool KMCCutin::GetCutinUnavailable() { return cutin_unavailable.load(); }
    void KMCCutin::SetCutinUnavailable(bool set) { cutin_unavailable.store(set); }

    bool KMCCutin::GetCutinFinished() { return cutin_finished.load(); }
    void KMCCutin::SetCutinFinished(bool set) { cutin_finished.store(set); }

    bool KMCCutin::GetFollowerCacheDataLoaded() { return follower_cache_data_loaded.load(); }
    void KMCCutin::SetFollowerCacheDataLoaded(bool set) { follower_cache_data_loaded.store(set); }

    int KMCCutin::PeekNextCutInID(std::string aaaakmctype) {
        auto findit = kmc_category_rand_map.find(aaaakmctype);
        if (findit == kmc_category_rand_map.end()) return -1;

        auto &rand_data = findit->second;

        if ((size_t)rand_data.offset < rand_data.rand_values.size()) {
            return rand_data.rand_values[rand_data.offset];
        }

        if (rand_data.category_indices) {
            std::vector<int> next_loop;
            RefreshRandValues(rand_data.size, rand_data.low, rand_data.high, rand_data.category_indices, next_loop);

            rand_data.rand_values.insert(rand_data.rand_values.end(), next_loop.begin(), next_loop.end());
            return rand_data.rand_values[rand_data.offset];
        }

        // ここには一つの要素の場合しかこない
        rand_data.offset = 0;
        return rand_data.rand_values[rand_data.offset];
    }

    int KMCCutin::GetCutInID(std::string aaaakmctype) {
        auto findit = kmc_category_rand_map.find(aaaakmctype);
        if (findit == kmc_category_rand_map.end()) return -1;

        auto *rand_data = &(findit->second);

        if (rand_data->offset > rand_data->maxIndex) {
            rand_data->rand_values.erase(rand_data->rand_values.begin(),
                                         rand_data->rand_values.begin() + rand_data->offset);
            rand_data->offset = 0;
        }

        int result = rand_data->rand_values[rand_data->offset];
        rand_data->offset++;

        return result;
    }

    void KMCCutin::RefreshRandValues(size_t size, int min, int max, const std::vector<int> *random_indices,
                                     std::vector<int> &out_values) {
        if (random_indices) {
            auto random_values = MakeRandArraySelect(size, min, max);

            out_values.reserve(random_values.size());
            for (int idx : random_values) {
                out_values.push_back(random_indices->at(idx));
            }
        }
    }

    void KMCCutin::PlayDuoCutin(KMCAnimST st) {
        int r = st.rand;
        std::string srnd = std::to_string(r);
        std::string timing = KMCCT::ST_AFTER;

        auto playback_priority = st.playback_priority;
        if (playback_priority != nullptr) {
            auto it = playback_priority->find(r);

            if (it != playback_priority->end()) {
                timing = it->second;
            }
        }
        std::map<int, KMCCutinOrder> *animmap = nullptr;

        if (timing == KMCCT::ST_BEFORE) {
            animmap = &AnimFtoAnimP;
        } else {
            animmap = &AnimPtoAnimF;
        }

        if (animmap == nullptr) {
            KMC_ERROR("not found cutin type");
            return;
        }

        // キャッシュモードが1のフォロワーがいる場合ここで画像読み込みをさせる
        int f_ackey = st.frand + 1;
        int cmode = KMCDisplayWordAndTexture::GetSingleton()->GetCacheModeMap(f_ackey);
        if (cmode == 1) {
            SetFollowerCacheDataLoaded(false);
            KMCPrismaUIBridge::GetSingleton()->KMCPreloadGroup(f_ackey, r);


            time_point<Clock> start_time = Clock::now();
            auto *thread = KMCCT::KMCEventThread::GetSingleton();

            while (!GetFollowerCacheDataLoaded()) {
                if (thread->GetForceEndAnim() || thread->IsShuttingDown()) return;

                auto now = Clock::now();
                auto wait_duration = duration_cast<milliseconds>(now - start_time).count();

                if (wait_duration >= READY_TIMEOUT) {
                    // 30秒経過したら待つのをやめる
                    KMC_ERROR("Cutin timeout: JS Ready didn't respond. ID: {} Category ID: {}", f_ackey, r);
                    throw std::runtime_error("[KMCOnCutinStartReady] Cutin response timeout");
                }

                std::this_thread::sleep_for(milliseconds(SE_PROGRESS_ADDTION_MS));
            }
        }

        auto *thread = KMCCT::KMCEventThread::GetSingleton();
        try {
            for (auto [key, value] : *animmap) {
                value.func(&st, value.is_player);
                if (thread->GetForceEndAnim() || thread->IsShuttingDown()) {
                    return;
                }
            }
        } catch (const std::exception &e) {
            // タイムアウト発生時
            // あってはならない
            KMC_ERROR("PlayDuoCutin aborted: {}", e.what());
        }
    }

    void KMCCutin::PlaySoloCutin(KMCAnimST st) {
        std::map<int, KMCCutinOrder> *animmap;

        // disp widget
        animmap = &AnimPlayerOnly;

        if (animmap == nullptr) {
            KMC_ERROR("not found cutin type");
            return;
        }

        auto *thread = KMCCT::KMCEventThread::GetSingleton();
        try {
            for (auto [key, value] : *animmap) {
                value.func(&st, value.is_player);
                if (thread->GetForceEndAnim() || thread->IsShuttingDown()) {
                    return;
                }
            }
        } catch (const std::exception &e) {
            // タイムアウト発生時
            // あってはならない
            KMC_ERROR("PlayDuoCutin aborted: {}", e.what());
        }
    }

    void KMCCutin::AnimationLoopSimple(const KMCAnimData &anim_data) {
        KMC_LOG("--------------KMCEventThread::AnimationLoop START------------");

        if (anim_data.cf.Sound) {
            KMCCT::KMCSound::GetSingleton()->PlayEx(anim_data.rand, anim_data.volum, anim_data.speaker,
                                                    anim_data.frand);
        }

        time_point<Clock> anim_start = Clock::now();
        auto *thread = KMCCT::KMCEventThread::GetSingleton();
        while (true) {
            if (thread->GetForceEndAnim() || thread->IsShuttingDown()) {
                return;
            }

            auto now = Clock::now();
            long long elapsed = duration_cast<milliseconds>(now - anim_start).count();

            // Prisma側から終了通知(KMCOnCutinFinished)が来た
            if (GetCutinFinished()) {
                KMC_LOG("Cutin finished by JS notification.");
                break;
            }

            // Prisma側からの通知が来ない場合の保険(設定時間 + 30秒)
            if (elapsed >= anim_data.time + READY_TIMEOUT) {
                KMC_ERROR("Cutin timeout: JS Finished didn't respond, forcing break.");
                throw std::runtime_error("[AnimationLoopSimple] Cutin response timeout");
            }

            // 負荷軽減のための待機
            std::this_thread::sleep_for(milliseconds(SE_PROGRESS_ADDTION_MS));
        }

        KMC_LOG("--------------KMCEventThread::AnimationLoop END------------");

        std::this_thread::sleep_for(std::chrono::milliseconds(SE_PROGRESS_ADDTION_MS));
    }

    void KMCCutin::AnimationLoopWithSE(const KMCAnimData &anim_data) {
        KMC_LOG("-------------- KMCCutin::AnimationLoopWithJSWait START ------------");

        if (anim_data.cf.Sound) {
            KMCCT::KMCSound::GetSingleton()->PlayEx(anim_data.rand, anim_data.volum, anim_data.speaker,
                                                    anim_data.frand);
        }

        std::string se_record = anim_data.record;
        time_point<Clock> anim_start_time = Clock::now();
        long long play_se_timer = 0;

        auto *thread = KMCCT::KMCEventThread::GetSingleton();

        while (true) {
            if (thread->GetForceEndAnim() || thread->IsShuttingDown()) return;

            if (GetCutinFinished()) {
                KMC_LOG("Cutin finished by JS notification.");
                break;
            }

            auto now = Clock::now();
            long long elapsed_limit = duration_cast<milliseconds>(now - anim_start_time).count();

            if (elapsed_limit >= anim_data.time + READY_TIMEOUT) {
                KMC_ERROR("Cutin timeout: JS Finished didn't respond.");
                throw std::runtime_error("[AnimationLoopWithSE] Cutin response timeout");
            }

            std::this_thread::sleep_for(milliseconds(SE_PROGRESS_ADDTION_MS));

            play_se_timer += SE_TIMER_INTERVAL_MS;
            if (se_record != "" && play_se_timer >= anim_data.time * std::stof(se_record)) {
                KMC_LOG(" time ms {} record {} dur {}", anim_data.time, se_record, play_se_timer);
                KMCCT::KMCSound::GetSingleton()->PlaySEEx(anim_data.rand, anim_data.frand, &se_record, anim_data.volum);
            }
        }

        // 1.0で設定されたSEの場合は再生されない可能性があるためここ
        if (se_record != "") {
            KMC_LOG(" time ms {} record {} dur {}", anim_data.time, se_record, play_se_timer);
            KMCCT::KMCSound::GetSingleton()->PlaySEEx(anim_data.rand, anim_data.frand, &se_record, anim_data.volum);
        }

        KMC_LOG("-------------- KMCCutin::AnimationLoopWithJSWait END ------------");

        std::this_thread::sleep_for(std::chrono::milliseconds(SE_PROGRESS_ADDTION_MS));
    }

    void KMCCutin::PlaySE(long long time, int trackid, int frand, std::string record, float volume) {
        time_point<Clock> start = Clock::now();
        long long now = 0;

        time_point<Clock> end;
        long long dur = 0;

        std::vector<int>::iterator anim;

        std::string nrecord = record;
        auto *thread = KMCCT::KMCEventThread::GetSingleton();

        while (true) {
            if (thread->GetForceEndAnim() || thread->IsShuttingDown()) {
                return;
            }

            end = Clock::now();
            milliseconds diff = duration_cast<milliseconds>(end - start);
            dur = diff.count();
            if (dur >= time) {
                break;
            }

            now = dur;
            if (nrecord != "" && now >= time * std::stof(nrecord)) {
                KMC_LOG(" time ms {} record {} dur {}", time, nrecord, now);
                KMCCT::KMCSound::GetSingleton()->PlaySEEx(trackid, frand, &nrecord, volume);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(KMCCT::ANIMATION_FRAME_MS));
        }
    }

    void KMCCutin::MCMSettingChange(std::vector<float> *floatArray) {
        aaaakmcvolum = (*floatArray)[0];
        aaaakmcAnimtime = (*floatArray)[1];
        aaaakmctime = (*floatArray)[2];
        aaaakmcCycle = (*floatArray)[3];
    }

#pragma region Randomizer Widget Function
    void KMCCutin::CategoryRandomizer() {
        kmc_category_rand_map.clear();
        kmc_category_first_values.clear();

        const auto &target_map =
            KMCDisplayWordAndTexture::GetSingleton()->GetCategoryRangeMap((int)KMCDisplayType::PLAYER);
        const auto &index_map =
            KMCDisplayWordAndTexture::GetSingleton()->GetCategoryIndexMap((int)KMCDisplayType::PLAYER);

        for (auto [key, value] : target_map) {
            int l = 0;
            int h = value - 1;
            size_t size = (h - l) + 1;
            const auto &category_indices = index_map.at(key);
            if (l == h) {
                int first_val = category_indices.at(0);
                kmc_category_rand_map.emplace(key, KMCRandomData(0, 0, {first_val}, h, l, 1, nullptr));
                kmc_category_first_values.push_back(first_val);
                continue;
            }

            std::vector<int> random_value = MakeRandArraySelect(size, l, h);

            std::vector<int> final_values;
            final_values.reserve(random_value.size());

            for (int idx : random_value) {
                final_values.push_back(category_indices.at(idx));
            }
            int fv = final_values.at(0);
            kmc_category_rand_map.emplace(
                key, KMCRandomData(0, h - l, std::move(final_values), h, l, size, &category_indices));
            kmc_category_first_values.push_back(fv);
        }
    }
#pragma endregion

#pragma region interrupt event
    void KMCCT::InterruptEventAddItem() {
        int32_t goldvalue = std::stoll(aaaakmcInterruptData.condition1);
        int32_t value = std::stoll(aaaakmcInterruptData.condition2);

        if (goldvalue >= value) {
            KMCCT::KMCCutin::GetSingleton()->IterCutIn(
                KMCCutinValues(aaaakmcInterruptData.type, aaaakmctime, aaaakmcAnimtime, aaaakmcvolum));
            //::KMCCT::KMCEventThread::GetSingleton()->CutInCreate({aaaakmcInterruptData.type});
        }
    }

    void KMCCT::InterruptEventOnHit() {
        std::string is_power_attack = aaaakmcInterruptData.condition1;
        std::string mod = aaaakmcInterruptData.condition2;
        std::string type = aaaakmcInterruptData.type;

        if (is_power_attack == "1") {
            KMCCT::KMCCutin::GetSingleton()->IterCutIn(
                KMCCutinValues(type + mod, aaaakmctime, aaaakmcAnimtime, aaaakmcvolum));
            //::KMCCT::KMCEventThread::GetSingleton()->CutInCreate({type + mod});
        } else {
            KMCCT::KMCCutin::GetSingleton()->IterCutIn(
                KMCCutinValues(type, aaaakmctime, aaaakmcAnimtime, aaaakmcvolum));
            //::KMCCT::KMCEventThread::GetSingleton()->CutInCreate({type});
        }
    }
#pragma endregion

#pragma region function pointer

    void KMCPlayAnim(KMCAnimST *st, int &is_player) {
        // bool isanim = true;

        // if (isanim) {
        KMCCompsFlag cf;
        std::string record = "0.0";
        CutinEntry entry;

        KMCAnimData anim_data;

        if (is_player == 0) {
            // player
            anim_data.rand = st->rand;
            anim_data.next_rand = st->p_next_rand;
            anim_data.volum = st->volum;
            anim_data.speaker = st->speakerp;
            anim_data.frand = -1;
            anim_data.time = st->time;
            anim_data.entry = st->t;
            anim_data.cf = st->pcf;
            anim_data.record = st->precord;
        } else {
            auto ap = st->speakerf_handle.get();
            if (ap) {
                anim_data.speaker = ap.get();
            } else {
                // カットイン待機中のフォロワーのほうが死んでゲームから除外された場合
                // 音声はならせないが、カットインだけ継続
                anim_data.speaker = nullptr;
            }

            anim_data.rand = st->rand;
            anim_data.next_rand = st->f_netx_rand;
            anim_data.volum = st->volum;

            anim_data.frand = st->frand;
            anim_data.time = st->time;
            anim_data.entry = st->ft;
            anim_data.cf = st->fcf;
            anim_data.record = st->frecord;
        }

        // play sound
        if (anim_data.cf.SE) {
            KMCCT::KMCCutin::GetSingleton()->AnimationLoopWithSE(anim_data);
        } else {
            KMCCT::KMCCutin::GetSingleton()->AnimationLoopSimple(anim_data);
        }
        //} else {
        // 使うかもしれないので残しとく
        // アニメーションではない場合
        // KMCPlay(st, is_player);
        //}
    }

    // void KMCPlay(KMCAnimST *st, int &is_player) {
    //     int r;
    //     float volum;
    //     RE::TESObjectREFR *target;
    //     int frand;
    //     long long time;
    //     KMCCompsFlag cf;
    //     std::string record = "";
    //     if (is_player == 0) {
    //         // player
    //         r = st->rand;
    //         volum = st->volum;
    //         target = st->speakerp;
    //         frand = -1;
    //         time = st->time;
    //         cf = st->pcf;
    //         record = st->precord;
    //     } else {

    //        r = st->rand;
    //        volum = st->volum;
    //        target = st->speakerf;
    //        frand = st->frand;
    //        time = st->time;
    //        cf = st->fcf;
    //        record = st->frecord;
    //    }

    //    if (cf.Sound) {
    //        KMCCT::KMCSound::GetSingleton()->PlayEx(r, volum, target, frand);
    //    }

    //    if (cf.SE) {
    //        KMCCT::KMCCutin::GetSingleton()->PlaySE(time, r, frand, record, volum);
    //    } else {
    //        KMCTimer(time);
    //    }
    //}

    void KMCOnCutinStartReady(KMCAnimST *st, int &is_player) {
        KMCCutin::GetSingleton()->SetCutinStartReady(false);
        KMCCutin::GetSingleton()->SetCutinFinished(false);
        KMCCutin::GetSingleton()->SetCutinUnavailable(false);

        long long time = 0;
        int id = 0;

        if (is_player == 0) {
            KMCPrismaUIBridge::GetSingleton()->KMCPlayPlayerCutin(st->rand, st->p_next_rand, st->player_name);

            time = st->time;
        } else {
            id = st->frand + 1;
            KMCPrismaUIBridge::GetSingleton()->KMCPlayFollowerCutin(st->frand + 1, st->rand, st->f_netx_rand,
                                                                    st->follower_name);
            time = st->ftime;
        }

        time_point<Clock> start_time = Clock::now();
        auto *thread = KMCCT::KMCEventThread::GetSingleton();

        while (!KMCCutin::GetSingleton()->GetCutinStartReady()) {
            if (thread->GetForceEndAnim() || thread->IsShuttingDown()) return;

            auto now = Clock::now();
            auto wait_duration = duration_cast<milliseconds>(now - start_time).count();

            if (wait_duration >= READY_TIMEOUT + time) {
                // 30秒＋カットイン時刻分経過したら待つのをやめる
                KMC_ERROR("Cutin timeout: JS Ready didn't respond. ID: {} Category ID: {}", id, st->rand);
                throw std::runtime_error("[KMCOnCutinStartReady] Cutin response timeout");
            }

            if (KMCCutin::GetSingleton()->GetCutinUnavailable()) {
                KMC_ERROR("Cutin skipped: JS reported unavailable (no frames). ID: {} Category ID: {}", id, st->rand);
                throw std::runtime_error("[KMCOnCutinStartReady] Cutin unavailable - possible missing assets");
            }

            std::this_thread::sleep_for(milliseconds(SE_PROGRESS_ADDTION_MS));
        }
    }

    void KMCOnCutinEnd(KMCAnimST *st, int &is_player) {
        KMCCutin::GetSingleton()->SetCutinStartReady(false);
        KMCCutin::GetSingleton()->SetCutinFinished(false);
        KMCCutin::GetSingleton()->SetCutinUnavailable(false);
    }

    void KMCOARFuncStart(KMCAnimST *st, int &is_player) {
        int frand;
        int rand;

        rand = st->rand;

        if (is_player == 0) {
            frand = -1;
        } else {
            frand = st->frand;
        }

        KMCCT::KMCOAR::GetSingleton()->PushOARFunc(rand, frand, st->overri_oar_time, st->oar_time);
    }

    void KMCExpFuncStart(KMCAnimST *st, int &is_player) {
        int frand;
        int rand;

        rand = st->exp_rand == -1 ? st->rand : st->exp_rand;

        if (is_player == 0) {
            frand = -1;
        } else {
            frand = st->frand;
        }

        KMCCT::KMCExpression::GetSingleton()->PushExpFunc(rand, frand, st->overri_exp_time, st->exp_time);
    }

    void KMCBatchPreloadGroups(KMCAnimST *st, int &playerorfollower) {
        // if (st->frand != -1) {
        //     KMCPrismaUIBridge::GetSingleton()->KMCBatchPreloadGroups((int)KMCDisplayType::PLAYER, st->p_next_rand,
        //                                                              st->frand + 1, st->f_netx_rand);
        // }
    }
#pragma endregion

}