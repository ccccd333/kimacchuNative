#pragma once
#include "KMCUtility.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>

namespace KMCCT {

    const std::string SD_PLUGIN_NAME = "KimachuuCutInVoiceForm.esp";

    // json path (common)
    const std::string COMMON_PATH = "Data\\skse\\plugins\\KimachuuCutIn\\";
    // prisma ui html path
    const std::string PRISMA_UI_HTML_PATH = "Data\\PrismaUI\\views\\KMCCutinPlugin\\";
    // json path (player or Follower1～n)
    const std::string PLAYER_WORD_PATH = "Player";
    const std::string FOLLOWER_WORD_PATH = "Follower";
    // json path nameplate
    const std::string NAMEPLATE_ANIM_PATH = "NamePlateAnimation";
    // json path profile
    const std::string PROFILE_PATH = "Profile";
    const std::string PROFILE_ANIM_PATH = "Animation";

    // speach timing tag
    const std::string ST_BEFORE = "before";
    const std::string ST_AFTER = "after";

    // common
    const std::string SETTING_FILE_NAME = "setting.json";
    const std::string MANAGED_FOLLOWER_FILE_NAME = "ManagedFollower.json";

    const std::string CUT_IN_COND_FILE_NAME = "Condition.json";

    const std::string INVISIBLE_TIMING_SETTING_FILE_NAME = "InvisibleTimingSetting.json";
    const std::string PROFILE_TEXT_FADE_FILE_NAME = "TextFade.json";
    const std::string PROFILE_SOUND_EFFECT_FILE_NAME = "SoundEffect.json";


    // follower
    const std::string SPEACH_TIMING_FILE_NAME = "SpeachTiming.json";

    // other
    const int WAIT_WIDGET_SIZE = 30;

    // picture path
    const std::string PICT_ROOT = "Data/Interface/exported/";
    const std::string PICT_PATH1 = "KimachuuCutIn";
    const std::string PICT_PATH2 = "cutin";
    const std::string PICT_TYPE = ".dds";
    const std::string NAME_PLATE_PICT_NAME = "NamePlate";
    const std::string PROFILE_PICT_NAME = "Profile";
    const std::string WAIT_WIDGET_PICT_NAME = "Wait";

    // sound descriptor (old)
    //const std::string LOAD_ORDER_ROOT = "aaaKimachuuCutInVFCategory";

    // json key name
    const std::string JSON_ROOT_KEY_STRING = "string";
    const std::string JSON_ROOT_KEY_INT = "int";

	class KMCConfig
	{
        SINGLETONHEADER(KMCConfig)
    public:
        void Setup();
        
        void Init();
        
        // common settings
        std::vector<std::pair<std::string, std::string>>* getISetting() { return &ISetting; }
        std::vector<std::pair<std::string, std::string>>* getIManagedFollower() { return &IManagedFollower; }    

        std::vector<std::pair<std::string, std::string>>* getIInvisibleTimingSetting() { return &IInvisibleTimingSetting; } 
        std::vector<std::pair<std::string, std::string>>* getIProfileSoundEffect() { return &IProfileSoundEffect; } 
        
        // Player only(It's too much trouble to modify the program...)        
                
        RE::PlayerCharacter* GetPlayer() { return player; }

        // Follower         
        std::vector<KMCFollower>* GetFollowers() { return &IFollower; }

        bool IsUnhookEnabled() const { return _unhook_papyrus; }
    private:
        bool SetupJsonSimpleNodes(std::vector<std::pair<std::string, std::string>> *configs, std::string jsonFileName,
                                  std::string rootKeyName);
        bool SetupJsonSimpleArray(std::vector<std::string>* configs, std::string jsonFileName,
                                  std::string rootKeyName);
        bool SetupJsonNestedNodes(std::vector<std::pair<std::string, std::map<std::string, std::string>>>* configs,
                                  std::string jsonFileName,
                                  std::string rootKeyName);
        std::string ISetup(std::string target, 
                           std::vector<std::pair<std::string, std::string>>* cond);

    private:

        // common
        std::vector<std::pair<std::string, std::string>> ISetting;
        std::vector<std::pair<std::string, std::string>> IManagedFollower;

        std::vector<std::pair<std::string, std::string>> IInvisibleTimingSetting;
        std::vector<std::pair<std::string, std::string>> IProfileSoundEffect;


        // Player
        std::vector<std::pair<std::string, std::string>> IConditions;

        RE::PlayerCharacter* player;
        
        // follower
        std::vector<KMCFollower> IFollower;

        bool _unhook_papyrus = true;
	};
}