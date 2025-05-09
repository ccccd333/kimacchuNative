#pragma once
#include "IWWFunctions.h"
#include "KMCCheckSource.h"
#include "KMCGameEventListener.h"
#include "KMCCategory.h"

namespace KMCCT {

    // class KMCCCheckTaskIF {
    // public:
    //     virtual ~KMCCCheckTaskIF() = default;
    //     // カウントを進めるためのIF
    //     virtual void Check() const = 0;
    //     virtual KMCCustomCondCheckHub GetDetail(KMCCCheckSource source) const = 0;
    // };

    class KMCCustomCondCheckHub {
    public:
        KMCCustomCondCheckHub() {}
        virtual ~KMCCustomCondCheckHub() {}
        virtual bool Init() { return false; }
        virtual bool Check(KMCCCheckSource source) { return false; }
        virtual void Reset() {}
        virtual std::unique_ptr<KMCCustomCondCheckHub> GetDetail(KMCCCheckSource source) { return nullptr; }
    };

    class KMCCCTNothing : public KMCCustomCondCheckHub {
    public:
        bool Init() override { return true; }
        bool Check(KMCCCheckSource source) override;
        void Reset() override {}
        std::unique_ptr<KMCCustomCondCheckHub> GetDetail(KMCCCheckSource source) override {
            if (source.main_category == KMCCCMainCategory::NOTHING) {
                return std::make_unique<KMCCCTNothing>();
            }

            return nullptr;
        }
    };

    class KMCCCTPlayerMove : public KMCCustomCondCheckHub {
    public:
        bool Init() override { return true; }
        bool Check(KMCCCheckSource source) override;
        void Reset() override {}
        std::unique_ptr<KMCCustomCondCheckHub> GetDetail(KMCCCheckSource source) override {
            if (source.main_category == KMCCCMainCategory::PLAYER) {
                if (source.sub1_category == KMCCCSubCategory::MOVE) {
                    return std::make_unique<KMCCCTPlayerMove>();
                }
            }

            return nullptr;
        }
    };

    class KMCCCTPlayerCombat : public KMCCustomCondCheckHub {
    public:
        bool Init() override { return true; }
        bool Check(KMCCCheckSource source) override;
        void Reset() override {}
        std::unique_ptr<KMCCustomCondCheckHub> GetDetail(KMCCCheckSource source) {
            if (source.main_category == KMCCCMainCategory::PLAYER) {
                if (source.sub1_category == KMCCCSubCategory::COMBAT) {
                    return std::make_unique<KMCCCTPlayerCombat>();
                }
            }

            return nullptr;
        }
    };

    class KMCCCTPlayerRunning : public KMCCustomCondCheckHub {
    public:
        bool Init() override { return true; }
        bool Check(KMCCCheckSource source) override;
        void Reset() override {}
        std::unique_ptr<KMCCustomCondCheckHub> GetDetail(KMCCCheckSource source) {
            if (source.main_category == KMCCCMainCategory::PLAYER) {
                if (source.sub1_category == KMCCCSubCategory::RUNNING) {
                    return std::make_unique<KMCCCTPlayerRunning>();
                }
            }

            return nullptr;
        }
    };

    class KMCCCTPlayerSneaking : public KMCCustomCondCheckHub {
    public:
        bool Init() override { return true; }
        bool Check(KMCCCheckSource source) override;
        void Reset() override {}
        std::unique_ptr<KMCCustomCondCheckHub> GetDetail(KMCCCheckSource source) {
            if (source.main_category == KMCCCMainCategory::PLAYER) {
                if (source.sub1_category == KMCCCSubCategory::SNEAKING) {
                    return std::make_unique<KMCCCTPlayerSneaking>();
                }
            }

            return nullptr;
        }
    };

    class KMCCCTPlayerIdle : public KMCCustomCondCheckHub {
    public:
        bool Init() override { return true; }
        bool Check(KMCCCheckSource source) override;
        void Reset() override {}
        std::unique_ptr<KMCCustomCondCheckHub> GetDetail(KMCCCheckSource source) {
            if (source.main_category == KMCCCMainCategory::PLAYER) {
                if (source.sub1_category == KMCCCSubCategory::IDLE) {
                    return std::make_unique<KMCCCTPlayerIdle>();
                }
            }

            return nullptr;
        }
    };

    class KMCCCTPlayerKeyword : public KMCCustomCondCheckHub {
    public:
        bool Init() override { return true; }
        bool Check(KMCCCheckSource source) override;
        void Reset() override {}
        std::unique_ptr<KMCCustomCondCheckHub> GetDetail(KMCCCheckSource source) {
            if (source.main_category == KMCCCMainCategory::PLAYER) {
                if (source.sub1_category == KMCCCSubCategory::KEYWORD) {
                    return std::make_unique<KMCCCTPlayerKeyword>();
                }
            }

            return nullptr;
        }
    };

    class KMCCCTLocationKeyword : public KMCCustomCondCheckHub {
    public:
        bool Init() override { return true; }
        bool Check(KMCCCheckSource source) override;
        void Reset() override {}
        std::unique_ptr<KMCCustomCondCheckHub> GetDetail(KMCCCheckSource source) {
            if (source.main_category == KMCCCMainCategory::LOCATION) {
                if (source.sub1_category == KMCCCSubCategory::KEYWORD) {
                    return std::make_unique<KMCCCTLocationKeyword>();
                }
            }

            return nullptr;
        }
    };

    class KMCCCTPlayerKill : public KMCCustomCondCheckHub {
    public:
        const std::string TIME_TO_INCLUDE_IN_KILL_COUNT = "time_to_include_in_kill_count";

        struct KillDetails {
        public:
            KillDetails() {}
            KillDetails(int count, time_point<Clock> now) {
                hit_count = count;
                latest_hit_time = now;
            }
            time_point<Clock> latest_hit_time;
            int hit_count = 0;
        };

        bool Init() override;
        bool Check(KMCCCheckSource source) override;
        void Reset() override { kill_count = 0; }
        std::unique_ptr<KMCCustomCondCheckHub> GetDetail(KMCCCheckSource source) {
            if (source.main_category == KMCCCMainCategory::PLAYER) {
                if (source.sub1_category == KMCCCSubCategory::KILL) {
                    // auto p = std::make_unique<KMCCCTPlayerKill>();
                    return std::make_unique<KMCCCTPlayerKill>();
                }
            }

            return nullptr;
        }

        void OnDeathEvent(const RE::TESDeathEvent* event);

        void OnHitEvent(const RE::TESHitEvent* event);

    private:
        
        int time_in_kill_count = 3;
        int kill_count = 0;
        std::map<unsigned long long, KillDetails> hit_and_kill_list;
        std::mutex mtx;
    };

    class KMCCCTTempKeyword : public KMCCustomCondCheckHub {
    public:
        bool Init() override { return true; }
        bool Check(KMCCCheckSource source) override;
        void Reset() override {}
        std::unique_ptr<KMCCustomCondCheckHub> GetDetail(KMCCCheckSource source) {
            if (source.main_category == KMCCCMainCategory::TEMP_KEYWORD) {
                
                return std::make_unique<KMCCCTTempKeyword>();
            }

            return nullptr;
        }
    };

    class KMCCCTCrossHair : public KMCCustomCondCheckHub {
    public:
        KMCCCTCrossHair() {}
        KMCCCTCrossHair(KMCCCheckSource souce) { 
            o_source = souce;
        }

        const std::string GET_CELL_NAME_TELEPORT_LINKED_DOOR = "GetCellNameTeleportLinkedDoor";
        const std::string GET_NAME = "GetName";

        const std::string NOT_FOUND = "<<<NOT FOUND>>>";

        std::set<std::string> tags = {"GetCellNameTeleportLinkedDoor", "GetName"};

        enum class kmc_pattern_mat {
            pattern,
            norm,
            none
        };

        enum class kmc_if { 
            get_cell_name_teleport_linked_door,
            get_name,
            none
        };

        struct KMCPattern {
        public:
            KMCPattern(){};

            std::regex pattern;
            std::string norm = "";
            kmc_pattern_mat kpm = kmc_pattern_mat::norm;
            kmc_if kmcif = kmc_if::none;

            std::string work = "";
        };

        bool Init() override;
        bool Check(KMCCCheckSource source) override;
        void Reset() override {}
        std::unique_ptr<KMCCustomCondCheckHub> GetDetail(KMCCCheckSource source) {
            if (source.main_category == KMCCCMainCategory::CROSS_HAIR) {
                return std::make_unique<KMCCCTCrossHair>(source);
            }

            return nullptr;
        }

    private:
        bool GetCellNameTeleportLinkedDoor(const KMCPattern& p, std::string& Obtained);
        bool GetName(const KMCPattern& p, std::string& Obtained);
        bool IsMatch(const std::string& name, const KMCPattern& p);

        KMCCCheckSource o_source;
        std::vector<KMCPattern> ifs;
        RE::CrosshairPickData* cpd = nullptr;
    };

    class KMCCCTBodySlot : public KMCCustomCondCheckHub {
    public:
        bool Init() override { return true; }
        bool Check(KMCCCheckSource source) override;
        void Reset() override {}
        std::unique_ptr<KMCCustomCondCheckHub> GetDetail(KMCCCheckSource source) override {
            if (source.main_category == KMCCCMainCategory::PLAYER) {
                if (source.sub1_category == KMCCCSubCategory::BODY_SLOT) {
                    return std::make_unique<KMCCCTBodySlot>();
                }
            }

            return nullptr;
        }
    };

    class KMCCCTMagicEffectKeyword : public KMCCustomCondCheckHub {
    public:
        bool Init() override { return true; }
        bool Check(KMCCCheckSource source) override;
        void Reset() override {}
        std::unique_ptr<KMCCustomCondCheckHub> GetDetail(KMCCCheckSource source) {
            if (source.main_category == KMCCCMainCategory::PLAYER) {
                if (source.sub1_category == KMCCCSubCategory::MAGIC_EFFECT_KEYWORD) {
                    return std::make_unique<KMCCCTMagicEffectKeyword>();
                }
            }

            return nullptr;
        }
    };

    bool GetCheckTaskDetail(KMCCCheckSource source, std::unique_ptr<KMCCustomCondCheckHub>* hub);

}