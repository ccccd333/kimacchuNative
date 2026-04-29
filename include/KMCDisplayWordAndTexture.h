#pragma once
#include "KMCUtility.h"

namespace KMCCT {

    class KMCDisplayWordAndTexture {
        SINGLETONHEADER(KMCDisplayWordAndTexture)
    public:
        const std::string DISPLAY_WORD_AND_TEXTURE_PATH = "DisplayWordAndTexture.json";

        void Init();

        bool IsReady() const { return loaded; }

        const std::unordered_map<int, std::string>& GetCategoryMap(int type) const {
            return category_map.at(type);
        }

        const std::unordered_map<std::string, int>& GetCategoryRangeMap(int type) const {
            return category_range_map.at(type);
        }

        const std::unordered_map<std::string, std::vector<int>>& GetCategoryIndexMap(int type) const {
            return category_index_map.at(type);
        }

        const std::unordered_map<int, CutinEntry>& GetEntriesDataMap(int type) const {
            return entries_data_map.at(type);
        }

        const int GetCacheModeMap(int type) const {
            return cache_mode_map.at(type);
        }

    private:
        bool Parse(std::string path, int disp_type, std::string actor_name);


        //{
        //	player(0):{
        //		1:"Idle",
        //		2:"Idle",
        //		3:"IdlePM",
        //		21:"Idle"
        //	},
        // follower(1):{},
        // follower(2):{}
        //}
        std::map<int, std::unordered_map<int, std::string>> category_map;

        //{
        //	player(0):{
        //		"Idle":3,
        //		"IdlePM":1
        //	}
        //}
        std::map<int, std::unordered_map<std::string, int>> category_range_map;

        //{
        //	player(0):{
        //		"Idle":[1,2,21],
        //		"IdlePM":[3]
        //	}
        //}

        // KMCCutin::CategoryRandomizerで取ってくる際indexがないと上から順番にIdleを定義していない場合
        // Randamが範囲でランダムに振れなくなってしまう(0~2とすればIdlePMが2になってしまうため)
        // そのためindexで最終的なカットインの番号を渡す
        // todo: CategoryRandomizer直すこと寝て忘れないこと
        std::map<int, std::unordered_map<std::string, std::vector<int>>> category_index_map;

        // cutin用にentryの中身を取っておく
        std::map<int, std::unordered_map<int, CutinEntry>> entries_data_map;

        // キャッシュモード用のマップ
        std::map<int, int> cache_mode_map;

        bool loaded = true;
    };

}