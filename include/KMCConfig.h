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
    // json path (player or Follower1Å`n)
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
    const std::string AUTO_WORD_RANGE_FILE_NAME = "AutoWordRange.json";
    const std::string AUTO_WORD_CATEG_FILE_NAME = "AutoWordCategories.json";
    const std::string MANAGED_FOLLOWER_FILE_NAME = "ManagedFollower.json";
    const std::string NAME_PLATE_FILE_NAME = "NamePlate.json";
    const std::string DETECTION_KEYWORD_FILE_NAME = "DetectionKeyword.json";
    const std::string DETECTION_FACTION_FILE_NAME = "DetectionFaction.json";
    const std::string DETECTION_MAGIC_EFFECT_KEYWORD_FILE_NAME = "DetectionMagicEffectKeyword.json";
    const std::string DETECTION_GLOBAL_FILE_NAME = "DetectionGlobal.json";
    const std::string DETECTION_STORAGE_UTIL_FILE_NAME = "DetectionStorageUtil.json";
    const std::string CUT_IN_COND_FILE_NAME = "Condition.json";

    const std::string INVISIBLE_TIMING_SETTING_FILE_NAME = "InvisibleTimingSetting.json";
    const std::string PROFILE_TEXT_FADE_FILE_NAME = "TextFade.json";
    const std::string PROFILE_SOUND_EFFECT_FILE_NAME = "SoundEffect.json";

    const std::map<int, std::string> NAMEPLATE_SETTINGS_PATH = {
        {(int)KMCWipeType::simply, "SimpleWipe.json"},
        {(int)KMCWipeType::end_fadeout, "NamePlateFadeOut.json"}
    };

    // player and follower
    const std::string AUTO_WORD_FILE_NAME = "AutoWord.json";
    const std::string AUTO_WORD_WF_FILE_NAME = "AutoWordAndWidgetConfigs.json";
    const std::string SOUND_DESC_FORMID_FILE_NAME = "VoiceSound.json";
    const std::string ANIM_RANGE_FILE_NAME = "AnimationRange.json";
    const std::string CONDITIONS_FILE_NAME = "ConditionsAndKeywords.json";
    const std::string SOUND_DESC_SE_FORMID_FILE_NAME = "SoundEffect.json";
    const std::string HIDE_COMPONENTS_FILE_NAME = "HideComponents.json";
    const std::string CONNECT_OAR_FILE_NAME = "OARComps.json";

    // player profil
    const std::string PROFILE_TEXT_FILE_NAME = "ProfileText.json";
    const std::string TEXT_SETTING_FILE_NAME = "TextSetting.json";
    const std::string WIDGET_SETTING_FILE_NAME = "WidgetSetting.json";

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
        ~KMCConfig() {}

        void Setup();
        
        void Init();
        
        // common settings
        std::vector<std::pair<std::string, std::string>>* getIAutoWordCategoriesConfigs() { return &IAutoWordCategoriesConfigs; }
        std::vector<std::pair<std::string, std::string>>* getIAutoWordRangeConfigs() { return &IAutoWordRangeConfigs; }
        std::vector<std::pair<std::string, std::string>>* getISetting() { return &ISetting; }
        std::vector<std::pair<std::string, std::string>>* getIManagedFollower() { return &IManagedFollower; }
        std::vector<std::pair<std::string, KMCNamePlate>>* getINamePlate() { return &INamePlate; }
        KMCNamePlateAnimationSettings* getINamePlateAnimation(int type) { return &(INamePlateAnimation[type]); }
        std::vector<std::pair<std::string, std::string>>* getIDetectionKeyword() { return &IDetectionKeyword; }
        std::vector<std::pair<std::string, std::string>>* getIDetectionFaction() { return &IDetectionFaction; }
        std::vector<std::pair<std::string, std::string>>* getIDetectionMagicEffectKeyword() { return &IDetectionMagicEffectKeyword; }
        std::vector<std::pair<std::string, std::string>>* getIDetectionGlobal() { return &IDetectionGlobal; }
        std::vector<std::pair<std::string, std::string>>* getIDetectionStorageUtil() { return &IDetectionStorageUtil; }     

        std::vector<std::pair<std::string, std::string>>* getIInvisibleTimingSetting() { return &IInvisibleTimingSetting; } 
        std::vector<std::pair<std::string, std::string>>* getIProfileAnimTextFade() { return &IProfileAnimTextFade; } 
        std::vector<std::pair<std::string, std::string>>* getIProfileSoundEffect() { return &IProfileSoundEffect; } 
        
        // Player only(It's too much trouble to modify the program...)        
        std::vector<std::pair<std::string, std::string>>* getIAutoWordWFConfigs() { return &IAutoWordWFConfigs; }
        std::vector<std::pair<std::string, std::string>>* getIAutoWordConfigs() { return &IAutoWordConfigs; }
        std::vector<std::pair<std::string, std::string>>* getIAnimationRange() { return &IAnimationRange; }

        std::vector<std::pair<std::string, KMCCompsFlag>>* getIHideComponents() { return &IHideComponents; }
        
        RE::PlayerCharacter* GetPlayer() { return player; }
        // Player Profile
        std::vector<std::pair<std::string, std::string>>* getIWidgetSetting() { return &IWidgetSetting; }
        std::vector<std::pair<std::string, std::string>>* getITextSetting() { return &ITextSetting; }
        std::vector<std::string>* getIProfileText() { return &IProfileText; }

        // Follower         
        std::vector<KMCFollower>* GetFollowers() { return &IFollower; }
    private:
        bool SetupJsonSimpleNodes(std::vector<std::pair<std::string, std::string>> *configs, std::string jsonFileName,
                                  std::string rootKeyName);
        bool SetupJsonSimpleArray(std::vector<std::string>* configs, std::string jsonFileName,
                                  std::string rootKeyName);
        bool SetupJsonNestedNodes(std::vector<std::pair<std::string, std::map<std::string, std::string>>>* configs,
                                  std::string jsonFileName,
                                  std::string rootKeyName);
        std::string ISetup(std::string target, std::vector<std::pair<std::string, std::string>>* awc,
                           std::vector<std::pair<std::string, std::string>>* awwfc,
                           std::vector<std::pair<std::string, std::string>>* ar,
                           std::vector<std::pair<std::string, std::string>>* cond,
                           std::vector<std::pair<std::string, KMCCompsFlag>>* hc);

        void ProfilSetup(std::string target, std::vector<std::pair<std::string, std::string>> *pws,
                         std::vector<std::pair<std::string, std::string>>* ts, std::vector<std::string>* pt);


    private:

        // common
        std::vector<std::pair<std::string, std::string>> ISetting;
        std::vector<std::pair<std::string, std::string>> IAutoWordRangeConfigs;
        std::vector<std::pair<std::string, std::string>> IAutoWordCategoriesConfigs;
        std::vector<std::pair<std::string, std::string>> IManagedFollower;
        std::vector<std::pair<std::string, std::string>> NamePlate;
        std::vector<std::pair<std::string, KMCNamePlate>> INamePlate;
        std::vector<KMCNamePlateAnimationSettings> INamePlateAnimation;
        std::vector<std::pair<std::string, std::string>> IDetectionKeyword;
        std::vector<std::pair<std::string, std::string>> IDetectionFaction;
        std::vector<std::pair<std::string, std::string>> IDetectionMagicEffectKeyword;
        std::vector<std::pair<std::string, std::string>> IDetectionGlobal;
        std::vector<std::pair<std::string, std::string>> IDetectionStorageUtil;

        std::vector<std::pair<std::string, std::string>> IInvisibleTimingSetting;
        std::vector<std::pair<std::string, std::string>> IProfileAnimTextFade;
        std::vector<std::pair<std::string, std::string>> IProfileSoundEffect;


        // Player
        std::vector<std::pair<std::string, std::string>> IAutoWordConfigs;
        std::vector<std::pair<std::string, std::string>> IAutoWordWFConfigs;
        std::vector<std::pair<std::string, std::string>> IAnimationRange;
        std::vector<std::pair<std::string, std::string>> IConditions;
        std::vector<std::pair<std::string, KMCCompsFlag>> IHideComponents;

        // Player profil
        std::vector<std::pair<std::string, std::string>> IWidgetSetting;
        std::vector<std::pair<std::string, std::string>> ITextSetting;
        std::vector<std::string> IProfileText;

        RE::PlayerCharacter* player;
        
        // follower
        std::vector<KMCFollower> IFollower;
	};
}