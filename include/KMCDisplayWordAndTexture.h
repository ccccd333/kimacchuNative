#pragma once
#include "KMCUtility.h"

namespace KMCCT {

    enum class KMCDisplayType{ PLAYER, FOLLOWER, UNK };

    class KMCDisplayWordAndTexture {
        SINGLETONHEADER(KMCDisplayWordAndTexture)
    public:
        const std::string DISPLAY_WORD_AND_TEXTURE_PATH = "DisplayWordAndTexture.json";

        void Init();

        bool IsLoaded() const { return loaded; }

    private:
        bool Parse(std::string path, KMCDisplayType disp_type, int follower_index = -1);


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

        bool loaded = false;
    };

}