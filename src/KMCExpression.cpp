#include "KMCExpression.h"

#include "KMCConfig.h"
#include "KMCEventThread.h"
#include "KMCDisplayAddon.h"
#include "KMCDisplayWordAndTexture.h"

SINGLETONBODY(KMCCT::KMCExpression)

namespace KMCCT {
    struct CaseInsensitiveCompare {
        bool operator()(const std::string& a, const std::string& b) const {
            return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
                                                [](char c1, char c2) { return tolower(c1) < tolower(c2); });
        }
    };

    using Clock = std::chrono::steady_clock;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::time_point;
    using namespace std::literals::chrono_literals;

    time_point<Clock> force_exp_st_time;
    std::map<std::string, KMCRandomData, CaseInsensitiveCompare> aaaakmcCategoryRandMap;

    bool BuildMFGValues(STMFGPair& pair, MFG_TYPE type, std::string tv) {

            auto sp = KMCSplit(tv, ',');
            if (sp.size() == 2) {
                int exp = std::stoi(sp[0]);
                int str = std::stoi(sp[1]);
                if (type == MFG_TYPE::modifier) {
                    if (exp < 0 || exp > 13) {
                        KMC_ERROR("modifier must be between 0 and 13");
                        return false;
                    }
                    if (str < 0 || str > 100) {
                        KMC_ERROR("modifier strength must be between 0 and 100");
                        return false;
                    }
                } else if (type == MFG_TYPE::phoneme) {
                    if (exp < 0 || exp > 15) {
                        KMC_ERROR("phoneme must be between 0 and 15");
                        return false;
                    }
                    if (str < 0 || str > 100) {
                        KMC_ERROR("phoneme strength must be between 0 and 100");
                        return false;
                    }
                } else {
                    if (exp < 0 || exp > 16) {
                        KMC_ERROR("expression must be between 0 and 16");
                        return false;
                    }
                    if (str < 0 || str > 100) {
                        KMC_ERROR("expression strength must be between 0 and 100");
                        return false;
                    }
                }

                pair.type.emplace_back((int)type);
                pair.exp.emplace_back(exp);
                pair.str.emplace_back(str);
            } else {
                KMC_ERROR("Fewer definitions.");
                return false;
            }

        return true;
    }

    void InitLoop(int actor_id, STMFG& result) {
        const auto* addons = KMCDisplayAddon::GetSingleton()->GetActorAddons(actor_id);

        // ƒAƒhƒIƒ“‚ª‘¶Ý‚µ‚È‚¢ê‡‚Í‰½‚à‚µ‚È‚¢
        if (!addons) return;

        try {
            for (const auto& [cutin_no, data] : addons->cutin_entries) {
                STMFGPair pair;
                const auto& exp = data.expression;

                for (const auto& v : exp.modifier) {
                    BuildMFGValues(pair, MFG_TYPE::modifier, v);
                }

                for (const auto& v : exp.phoneme) {
                    BuildMFGValues(pair, MFG_TYPE::phoneme, v);
                }

                if (!exp.expression.empty()) {
                    BuildMFGValues(pair, MFG_TYPE::expression, exp.expression);
                }

                pair.time = exp.time;

                result.mfg[static_cast<uint64_t>(cutin_no)] = pair;
            }
        } catch (const std::exception& e) {
            SKSE::log::error("InitLoop error: actor_id={}, what={}", actor_id, e.what());
        }
    }

    void KMCExpression::Setup() {
        // player
        InitLoop((int)KMCDisplayType::PLAYER, player_mfg);

        // follower
        auto followers = KMCCT::KMCConfig::GetSingleton()->GetFollowers();
        for (const auto& fs : *followers) {
            STMFG st_mfg;
            InitLoop(fs.index + 1, st_mfg);
            follower_mfg.emplace(fs.index, st_mfg);
        }

        papyrus_end_exp = false;
        exp_loop_now = false;
        f_papyrus_end_exp = false;
        f_exp_loop_now = false;
        force_exp_loop = false;
        force_cool_time = 0.0f;
        force_exp_time = 0.0f;
    }

    void KMCExpression::Init() { 
        form = RE::TESDataHandler::GetSingleton()->LookupForm(0x806, "KimachuuCutIn.esp"); 
        CategoryRandomizer();
    }

    int KMCExpression::OnStandby() {
        { 
            std::lock_guard<std::mutex> lock(wit_mtx);
            is_wait = true;
        }

        bool eln = false;
        {
            std::lock_guard<std::mutex> lock(mtx);
            eln = exp_loop_now;
        }

        bool feln = false;
        {
            std::lock_guard<std::mutex> lock(f_mtx);
            feln = f_exp_loop_now;
        }

        if (eln || feln) {
            auto* thread = KMCCT::KMCEventThread::GetSingleton();
            while (eln || feln) {
                if (thread->forceendanim || thread->IsShuttingDown()) {
                    return -1;
                }

                KMC_LOG("KMCExpression::OnStandby Wait");
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    eln = exp_loop_now;
                }

                {
                    std::lock_guard<std::mutex> lock(f_mtx);
                    feln = f_exp_loop_now;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(EXP_WAIT_LOOP));
            }
        }

        {
            std::lock_guard<std::mutex> lock(wit_mtx);
            // force exp
            force_cool_time = 0.0f;
            is_cutin = true;
        }

        return 0;
    }

    void KMCExpression::OnFinished() {
        {
            std::lock_guard<std::mutex> lock(wit_mtx);
            is_wait = false;
            is_cutin = false;
        }
    }

    void KMCExpression::PushExpFunc(int rand, int frand, bool force, float ex_exp_time) {
        if (rand == -1) {
            KMC_ERROR("KMCExpression::PushExpFunc rand == -1");
        }

        if (frand == -1) {
            PPushExpFunc(rand, force, ex_exp_time);
        } else {
            FPushExpFunc(rand, frand, force, ex_exp_time);
        }
    }
    void KMCExpression::TryKMCExp(STMFGPair* mfg) {

        auto* thread = KMCCT::KMCEventThread::GetSingleton();
        if (f_now.ac && p_now.ac) {
            if (f_now.ac->GetFormID() == p_now.ac->GetFormID()) {
                bool end = false;

                {
                    std::lock_guard<std::mutex> lock(f_mtx);
                    end = f_exp_loop_now;
                }

                if (end) {
                    KMC_LOG("KMCExpression Player == Follower(Player) Conflict");

                    int loop_count = 1;
                    while (end) {
                        if (GetWaitFlag()) {
                            {
                                std::lock_guard<std::mutex> lock(mtx);
                                exp_loop_now = false;
                            }
                            return;
                        }

                        if (thread->forceendanim || thread->IsShuttingDown()) {
                            return;
                        }

                        KMC_LOG("KMCExpression Player == Follower(Player) Conflict. Loop{}", loop_count);

                        {
                            std::lock_guard<std::mutex> lock(f_mtx);
                            end = f_exp_loop_now;
                        }

                        std::this_thread::sleep_for(std::chrono::milliseconds(EXP_NOT_END_LOOP));
                        loop_count++;
                    }
                    KMC_LOG("KMCExpression Player == Follower(Player) Conflict. End");
                }
            }
        }

        if (GetWaitFlag()) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                exp_loop_now = false;
            }
            return;
        }

        if (thread->forceendanim || thread->IsShuttingDown()) {
            return;
        }

        PapyrusFuncCall("aaaKimachuuCutInMCMScripts", "KimachuuExpression", form, mfg->ac, mfg->type, mfg->exp,
                        mfg->str);

        if (KMCEXPTimer(mfg->time * KMCCT::WHILE_WAIT_TIME) < 0) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                exp_loop_now = false;
            }
            return;
        }

        bool end = false;

        {
            std::lock_guard<std::mutex> lock(mtx);
            end = papyrus_end_exp;
        }

        if (!end) {
            while (!end) {
                if (thread->forceendanim || thread->IsShuttingDown()) {
                    return;
                }

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    end = papyrus_end_exp;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(EXP_NOT_END_LOOP));
            }
        }

        if (thread->forceendanim || thread->IsShuttingDown()) {
            return;
        }

        PapyrusFuncCall("aaaKimachuuCutInMCMScripts", "KimachuuStopExpression", form, mfg->ac);

        {
            std::lock_guard<std::mutex> lock(mtx);
            exp_loop_now = false;
        }

        KMC_LOG("KMCExpression Player End");
    }

    void KMCExpression::TryKMCFLExp(STMFGPair* mfg) {
        auto* thread = KMCCT::KMCEventThread::GetSingleton();
        if (f_now.ac && p_now.ac) {
            if (f_now.ac->GetFormID() == p_now.ac->GetFormID()) {
                bool end = false;

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    end = exp_loop_now;
                }

                if (end) {
                    KMC_LOG("KMCExpression Follower(Player) == Player Conflict");
                    int loop_count = 1;
                    while (end) {
                        if (GetWaitFlag()) {
                            {
                                std::lock_guard<std::mutex> lock(mtx);
                                exp_loop_now = false;
                            }
                            return;
                        }

                        if (thread->forceendanim || thread->IsShuttingDown()) {
                            return;
                        }

                        KMC_LOG("KMCExpression Follower(Player) == Player Conflict. Loop{}", loop_count);

                        {
                            std::lock_guard<std::mutex> lock(mtx);
                            end = exp_loop_now;
                        }

                        std::this_thread::sleep_for(std::chrono::milliseconds(EXP_NOT_END_LOOP));
                        loop_count++;
                    }

                    KMC_LOG("KMCExpression Follower(Player) == Player Conflict. End");
                }
            }
        }

        if (GetWaitFlag()) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                exp_loop_now = false;
            }
            return;
        }

        if (thread->forceendanim || thread->IsShuttingDown()) {
            return;
        }

        PapyrusFuncCall("aaaKimachuuCutInMCMScripts", "KimachuuFLExpression", form, mfg->ac, mfg->type, mfg->exp,
                        mfg->str);

        if (KMCEXPTimer(mfg->time * KMCCT::WHILE_WAIT_TIME) < 0) {
            {
                std::lock_guard<std::mutex> lock(f_mtx);
                f_exp_loop_now = false;
            }
            return;
        }

        bool end = false;

        {
            std::lock_guard<std::mutex> lock(f_mtx);
            end = f_papyrus_end_exp;
        }

        if (!end) {
            while (!end) {
                if (thread->forceendanim || thread->IsShuttingDown()) {
                    return;
                }

                {
                    std::lock_guard<std::mutex> lock(f_mtx);
                    end = f_papyrus_end_exp;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(EXP_NOT_END_LOOP));
            }
        }

        if (thread->forceendanim || thread->IsShuttingDown()) {
            return;
        }

        PapyrusFuncCall("aaaKimachuuCutInMCMScripts", "KimachuuStopExpression", form, mfg->ac);

        {
            std::lock_guard<std::mutex> lock(f_mtx);
            f_exp_loop_now = false;
        }

        KMC_LOG("KMCExpression Follower End");
    }

    void KMCExpression::SetEndPapurusExp() {
        KMC_LOG("KMCExpression Player Papyrus End");

        {
            std::lock_guard<std::mutex> lock(mtx);
            papyrus_end_exp = true;
        }
    }

    void KMCExpression::SetFLEndPapurusExp() {
        KMC_LOG("KMCExpression Follower Papyrus End");
        {
            std::lock_guard<std::mutex> lock(f_mtx);
            f_papyrus_end_exp = true;
        }
    }

    int KMCExpression::ForceExp(std::string exp_id, float cool_time, float exp_time) {

        bool loop_now = true;
        {
            std::lock_guard<std::mutex> lock(mtx);
            loop_now = exp_loop_now;
        }
        if (loop_now || GetWaitFlag()) {
            return -1;
        }

        time_point<Clock> end;
        long long dur = 0;
        // sleep
        if (force_cool_time != 0.0f) {
            KMC_LOG("KMCExpression::ForceExp cool time now {} {} {}", exp_id, cool_time, exp_time);
            end = Clock::now();
            
            milliseconds diff = duration_cast<milliseconds>(end - force_exp_st_time);
            dur = diff.count();
            if (dur <= force_cool_time * TIME_SCALE_MS) {
                return -2;
            }
        }

        auto rand = GetCutInID(exp_id);

        if (rand == 0) {
            return 1;
        }

        force_exp_st_time = Clock::now();
        force_cool_time = cool_time;
        force_exp_time = exp_time;

        PPushExpFunc(rand, true, force_exp_time);

        return 0;
    }

    void KMCExpression::PPushExpFunc(int rand, bool force, float ex_exp_time) {
        bool loop_now = true;
        {
            std::lock_guard<std::mutex> lock(mtx);
            loop_now = exp_loop_now;
        }
        if (loop_now) {
            return;
        }

        if (player_mfg.mfg.contains(rand)) {
            p_now = player_mfg.mfg.at(rand);
            if (p_now.type.size() > 0) {
                papyrus_end_exp = false;
                exp_loop_now = true;

                p_now.ac = KMCCT::KMCConfig::GetSingleton()->GetPlayer();
                if (ex_exp_time != 0.0f) {
                    p_now.time = ex_exp_time;
                }
                LaunchExp(p_now);
            }
        }
    }

    void KMCExpression::FPushExpFunc(int rand, int frand, bool force, float ex_exp_time) {
        bool loop_now = true;
        {
            std::lock_guard<std::mutex> lock(f_mtx);
            loop_now = f_exp_loop_now;
        }
        if (loop_now) {
            return;
        }

        if (follower_mfg.contains(frand)) {
            auto fmfg = follower_mfg.at(frand);

            auto followers = KMCCT::KMCConfig::GetSingleton()->GetFollowers();
            auto actor = followers->at(frand).follower;
            if (actor) {
                if (fmfg.mfg.contains(rand)) {
                    f_now = fmfg.mfg.at(rand);
                    if (f_now.type.size() > 0) {
                        f_papyrus_end_exp = false;
                        f_exp_loop_now = true;
                        f_now.ac = actor;

                        if (ex_exp_time != 0.0f) {
                            f_now.time = ex_exp_time;
                        }
                        LaunchFLExp(f_now);
                    }
                }
            }
        }
    }

    bool KMCExpression::GetWaitFlag() {
        {
            std::lock_guard<std::mutex> lock(wit_mtx);
            return is_wait == true && (is_cutin == false);
        }
    }

    int KMCExpression::KMCEXPTimer(long long limit) {
        time_point<Clock> start = Clock::now();
        time_point<Clock> end;
        long long dur = 0;
        auto* thread = KMCCT::KMCEventThread::GetSingleton();
        // sleep
        while (true) {
            if (GetWaitFlag()) {
                return -1;
            }

            if (thread->forceendanim || thread->IsShuttingDown()) {
                return -2;
            }
            end = Clock::now();
            milliseconds diff = duration_cast<milliseconds>(end - start);
            dur = diff.count();
            if (dur >= limit) {
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(EXP_TIMER_LOOP));
        }

        return 0;
    }

    int KMCExpression::GetCutInID(std::string aaaakmctype) {
        int rand = -1;
        auto findit = kmc_category_rand_map.find(aaaakmctype);
        if (findit != kmc_category_rand_map.end()) {
            auto* randData = &(findit->second);
            int of = randData->offset;
            auto r = randData->rand_values;
            rand = r[of];
            ++of;
            int mi = randData->maxIndex;
            if (of > mi) {
                of = 0;
                //auto randomValue = MakeRandArraySelect(randData->size, randData->low, randData->high);
                //randData->rand_values = std::move(randomValue);
            }

            randData->offset = of;
        }
        return rand;
    }

    void KMCExpression::CategoryRandomizer() {
        kmc_category_rand_map.clear();
        kmc_category_first_values.clear();

        int id = (int)KMCDisplayType::PLAYER;

        const auto& target_map = KMCDisplayWordAndTexture::GetSingleton()->GetCategoryRangeMap(id);
        const auto& index_map = KMCDisplayWordAndTexture::GetSingleton()->GetCategoryIndexMap(id);

        for (auto [key, value] : target_map) {
            int l = 0;
            int h = value - 1;
            size_t size = (h - l) + 1;
            auto &category_indices = index_map.at(key);
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

}