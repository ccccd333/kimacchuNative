#include "KMCExpression.h"

#include <nlohmann/json.hpp>

#include "KMCConfig.h"
#include "KMCEventThread.h"

SINGLETONBODY(KMCCT::KMCExpression)

namespace KMCCT {
    struct CaseInsensitiveCompare {
        bool operator()(const std::string& a, const std::string& b) const {
            return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
                                                [](char c1, char c2) { return tolower(c1) < tolower(c2); });
        }
    };

    using json = nlohmann::json;
    using Clock = std::chrono::steady_clock;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::time_point;
    using namespace std::literals::chrono_literals;

    time_point<Clock> force_exp_st_time;
    std::map<std::string, KMCRandomData, CaseInsensitiveCompare> aaaakmcCategoryRandMap;

    bool BuildMFGValues(STMFGPair& pair, MFG_TYPE type, json items) {
        for (auto ite = items.begin(); ite != items.end(); ite++) {
            std::string v = ite->get<std::string>();
            auto sp = KMCSplit(v, ',');
            if (sp.size() == 2) {
                int exp = std::stoi(sp[0]);
                int str = std::stoi(sp[1]);
                if (type == MFG_TYPE::modifier) {
                    if (exp < 0 || exp > 13) {
                        ERROR("modifier must be between 0 and 13");
                        return false;
                    }
                    if (str < 0 || str > 100) {
                        ERROR("modifier strength must be between 0 and 100");
                        return false;
                    }
                } else if (type == MFG_TYPE::phoneme) {
                    if (exp < 0 || exp > 15) {
                        ERROR("phoneme must be between 0 and 15");
                        return false;
                    }
                    if (str < 0 || str > 100) {
                        ERROR("phoneme strength must be between 0 and 100");
                        return false;
                    }
                } else {
                    if (exp < 0 || exp > 16) {
                        ERROR("expression must be between 0 and 16");
                        return false;
                    }
                    if (str < 0 || str > 100) {
                        ERROR("expression strength must be between 0 and 100");
                        return false;
                    }
                }

                pair.type.emplace_back((int)type);
                pair.exp.emplace_back(exp);
                pair.str.emplace_back(str);
            } else {
                ERROR("Fewer definitions.");
                return false;
            }
        }

        return true;
    }

    void InitLoop(std::string path, STMFG& result) {
        if (!IsFileExist(path)) {
            LOG("Expression.json not found {}", path);
            return;
        }

        try {
            std::ifstream stream(path);

            if (!stream.is_open()) throw new std::exception("Failed open file.");

            if (!json::accept(stream)) throw new std::exception("Incorrect json format.");

            stream.seekg(0, std::ios::beg);

            json j = json::parse(stream);

            for (auto& [key, value] : j.items()) {
                STMFGPair mfg_pair;
                for (auto& [mfg_types, mfg_var] : value.items()) {
                    if (mfg_types == "modifier") {
                        if (!BuildMFGValues(mfg_pair, MFG_TYPE::modifier, mfg_var)) {
                            ERROR("");
                        }
                    } else if (mfg_types == "phoneme") {
                        if (!BuildMFGValues(mfg_pair, MFG_TYPE::phoneme, mfg_var)) {
                            ERROR("");
                        }
                    } else if (mfg_types == "expression") {
                        if (!BuildMFGValues(mfg_pair, MFG_TYPE::expression, mfg_var)) {
                            ERROR("");
                        }
                    } else if (mfg_types == "time") {
                        mfg_pair.time = mfg_var.get<int>();
                    } else {
                        ERROR("");
                    }
                }
                result.mfg.emplace(std::stoll(key), mfg_pair);
            }
        } catch (std::exception ex) {
            ERROR("KMCExpression::Setup unkown exception wt:{}", ex.what());
        }
    }

    void KMCExpression::Setup() {
        // player
        InitLoop(COMMON_PATH + PLAYER_WORD_PATH + "\\" + EXPRESSION_FILE_NAME, player_mfg);

        // follower
        int i = 0;
        auto followers = KMCCT::KMCConfig::GetSingleton()->getFollowers();
        for (auto fs : *followers) {
            STMFG st_mfg;
            std::string fkey = std::to_string(i + 1);
            InitLoop(COMMON_PATH + FOLLOWER_WORD_PATH + fkey + "\\" + EXPRESSION_FILE_NAME, st_mfg);
            follower_mfg.emplace(i, st_mfg);
            i++;
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

    void KMCExpression::PushExpFunc(uint64_t rand, uint64_t frand, bool force, float ex_exp_time) {
        if (rand == -1) {
            ERROR("KMCExpression::PushExpFunc rand == -1");
        }

        if (frand == -1) {
            PPushExpFunc(rand, force, ex_exp_time);
        } else {
            FPushExpFunc(rand, frand, force, ex_exp_time);
        }
    }
    void KMCExpression::TryKMCExp(STMFGPair* mfg) {
        if (f_now.ac && p_now.ac) {
            if (f_now.ac->GetFormID() == p_now.ac->GetFormID()) {
                bool end = false;

                {
                    std::lock_guard<std::mutex> lock(f_mtx);
                    end = f_exp_loop_now;
                }

                if (end) {
                    LOG("KMCExpression Player == Follower(Player) Conflict");

                    int loop_count = 1;
                    while (end) {
                        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                            return;
                        }

                        LOG("KMCExpression Player == Follower(Player) Conflict. Loop{}", loop_count);

                        {
                            std::lock_guard<std::mutex> lock(f_mtx);
                            end = f_exp_loop_now;
                        }

                        std::this_thread::sleep_for(std::chrono::milliseconds(EXP_NOT_END_LOOP));
                        loop_count++;
                    }
                    LOG("KMCExpression Player == Follower(Player) Conflict. End");
                }
            }
        }

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return;
        }

        PapyrusFuncCall("aaaKimachuuCutInMCMScripts", "KimachuuExpression", form, mfg->ac, mfg->type, mfg->exp,
                        mfg->str);

        KMCCT::KMCTimer(mfg->time * KMCCT::WHILE_WAIT_TIME);

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return;
        }

        bool end = false;

        {
            std::lock_guard<std::mutex> lock(mtx);
            end = papyrus_end_exp;
        }

        if (!end) {
            while (!end) {
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                    return;
                }

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    end = papyrus_end_exp;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(EXP_NOT_END_LOOP));
            }
        }

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return;
        }

        PapyrusFuncCall("aaaKimachuuCutInMCMScripts", "KimachuuStopExpression", form, mfg->ac);

        {
            std::lock_guard<std::mutex> lock(mtx);
            exp_loop_now = false;
        }

        LOG("KMCExpression Player End");
    }

    void KMCExpression::TryKMCFLExp(STMFGPair* mfg) {
        
        if (f_now.ac && p_now.ac) {
            if (f_now.ac->GetFormID() == p_now.ac->GetFormID()) {
                bool end = false;

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    end = exp_loop_now;
                }

                if (end) {
                    LOG("KMCExpression Follower(Player) == Player Conflict");
                    int loop_count = 1;
                    while (end) {
                        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                            return;
                        }

                        LOG("KMCExpression Follower(Player) == Player Conflict. Loop{}", loop_count);

                        {
                            std::lock_guard<std::mutex> lock(mtx);
                            end = exp_loop_now;
                        }

                        std::this_thread::sleep_for(std::chrono::milliseconds(EXP_NOT_END_LOOP));
                        loop_count++;
                    }

                    LOG("KMCExpression Follower(Player) == Player Conflict. End");
                }
            }
        }

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return;
        }

        PapyrusFuncCall("aaaKimachuuCutInMCMScripts", "KimachuuFLExpression", form, mfg->ac, mfg->type, mfg->exp,
                        mfg->str);

        KMCCT::KMCTimer(mfg->time * KMCCT::WHILE_WAIT_TIME);

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return;
        }

        bool end = false;

        {
            std::lock_guard<std::mutex> lock(f_mtx);
            end = f_papyrus_end_exp;
        }

        if (!end) {
            while (!end) {
                if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
                    return;
                }

                {
                    std::lock_guard<std::mutex> lock(f_mtx);
                    end = f_papyrus_end_exp;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(EXP_NOT_END_LOOP));
            }
        }

        if (KMCCT::KMCEventThread::GetSingleton()->forceendanim) {
            return;
        }

        PapyrusFuncCall("aaaKimachuuCutInMCMScripts", "KimachuuStopExpression", form, mfg->ac);

        {
            std::lock_guard<std::mutex> lock(f_mtx);
            f_exp_loop_now = false;
        }

        LOG("KMCExpression Follower End");
    }

    void KMCExpression::SetEndPapurusExp() {
        LOG("KMCExpression Player Papyrus End");

        {
            std::lock_guard<std::mutex> lock(mtx);
            papyrus_end_exp = true;
        }
    }

    void KMCExpression::SetFLEndPapurusExp() {
        LOG("KMCExpression Follower Papyrus End");
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
        if (loop_now) {
            return -1;
        }

        time_point<Clock> end;
        long long dur = 0;
        // sleep
        if (force_cool_time != 0.0f) {
            LOG("KMCExpression::ForceExp cool time now {} {} {}", exp_id, cool_time, exp_time);
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

    void KMCExpression::PPushExpFunc(uint64_t rand, bool force, float ex_exp_time) {
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

                p_now.ac = KMCCT::KMCConfig::GetSingleton()->getPlayer();
                if (ex_exp_time != 0.0f) {
                    p_now.time = ex_exp_time;
                }
                LaunchExp(p_now);
            }
        }
    }

    void KMCExpression::FPushExpFunc(uint64_t rand, uint64_t frand, bool force, float ex_exp_time) {
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

            auto followers = KMCCT::KMCConfig::GetSingleton()->getFollowers();
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

    uint64_t KMCExpression::GetCutInID(std::string aaaakmctype) {
        uint64_t rand = 0;
        auto findit = aaaakmcCategoryRandMap.find(aaaakmctype);
        if (findit != aaaakmcCategoryRandMap.end()) {
            auto* randData = &(findit->second);
            int of = randData->offset;
            auto r = randData->randValues;
            rand = r[of];
            ++of;
            int mi = randData->maxIndex;
            if (of > mi) {
                of = 0;
                //auto randomValue = MakeRandArraySelect(randData->size, randData->low, randData->high);
                //randData->randValues = std::move(randomValue);
            }

            randData->offset = of;
        }
        return rand;
    }

    void KMCExpression::CategoryRandomizer() {
        aaaakmcCategoryRandMap.clear();

        std::vector<std::pair<std::string, std::string>>* autoWordRangeConfigs =
            KMCCT::KMCConfig::GetSingleton()->getIAutoWordRangeConfigs();
        std::vector<std::pair<std::string, std::string>>* autoWordCategoriesConfigs =
            KMCCT::KMCConfig::GetSingleton()->getIAutoWordCategoriesConfigs();

        // animation widget
        for (auto [key, value] : *autoWordCategoriesConfigs) {
            auto findit = aaaakmcCategoryRandMap.find(value);
            if (findit != aaaakmcCategoryRandMap.end()) {
                continue;
            }

            std::string k1 = value;
            std::string k2 = k1;
            std::transform(k1.begin(), k1.end(), k2.begin(), [](char c) { return std::tolower(c); });
            std::string tLow = k2 + "low";
            std::string tHigh = k2 + "high";
            auto it1 = std::find_if(autoWordRangeConfigs->begin(), autoWordRangeConfigs->end(),
                                    [tLow](const auto& p) { return p.first == tLow; });
            auto it2 = std::find_if(autoWordRangeConfigs->begin(), autoWordRangeConfigs->end(),
                                    [tHigh](const auto& p) { return p.first == tHigh; });
            if (it1 != autoWordRangeConfigs->end() && it2 != autoWordRangeConfigs->end()) {
                int l = std::stoi(it1->second);
                int h = std::stoi(it2->second);

                if ((h - l) + 1 <= 0) {
                    ERROR("Error KMCEventThread::CategoryRandomizer range error type = {} low = {} high = {}", k2, tLow,
                          tHigh);
                }

                size_t size = (h - l) + 1;
                auto randomValue = MakeRandArraySelect(size, l, h);

                aaaakmcCategoryRandMap.insert(std::make_pair(value, KMCRandomData(0, h - l, randomValue, h, l, size)));

                 for (int i = 0; i < randomValue.size(); i++) {
                     LOG("KMCEventThread::CategoryRandomizer type = {} rand = {}", value, randomValue[i]);
                 }

            } else {
                ERROR("Error KMCEventThread::CategoryRandomizer auto word range configs type = {} low = {} high = {}",
                      k2, tLow, tHigh);
            }
        }
    }
}