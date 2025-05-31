//#include "hooks.h"
//#include "IWWFunctions.h"
//#include "KMCStateManager.h"
//
//SINGLETONBODY(KMCCT::DialogueItemConstructorCall)
//namespace KMCCT {
//	bool Install() {
//		SKSE::AllocTrampoline(14);
//		LOG("Installing hooks, allocated 14 bytes to the trampoline.");
//
//		bool nominal = true;
//		auto* dialogueItemConstructorManager = DialogueItemConstructorCall::GetSingleton();
//		if (!dialogueItemConstructorManager) {
//			ERROR("  >Failed to get manager singleton for the Dialogue Item Ctor manager.");
//			nominal = false;
//		}
//
//		if (!nominal) {
//			return false;
//		}
//
//		bool installedCtorPatch = dialogueItemConstructorManager->Install();
//
//		return installedCtorPatch;
//	}
//
//	bool DialogueItemConstructorCall::Install()
//	{
//		auto& trampoline = SKSE::GetTrampoline();
//
//		REL::Relocation<std::uintptr_t> target{RELOCATION_ID(25014, 25541), 0xE2};
//		if (!REL::make_pattern<"E8">().match(target.address())) {
//			ERROR("  >Failed to match pattern for SE = 25014 + 0xE2 AE = 25541 + 0xE2.");
//			return false;
//		}
//		_func = trampoline.write_call<5>(target.address(), &Thunk);
//		return true;
//	}
//
//	RE::DialogueItem* DialogueItemConstructorCall::Thunk(
//		RE::DialogueItem* a_this, 
//		RE::TESQuest* a_quest,
//		RE::TESTopic* a_topic,
//		RE::TESTopicInfo* a_topicInfo,
//		RE::TESObjectREFR* a_speaker)
//	{
//		auto response = _func(a_this, a_quest, a_topic, a_topicInfo, a_speaker);
//		auto* speaker = a_speaker ? a_speaker->As<RE::Actor>() : nullptr;
//		if (!response || 
//			!speaker || 
//			!a_topic)
//		{
//            KMCCT::KMCStateManager::GetSingleton()->AllowDialogue(speaker, a_topic);
//		}
//        return response;
//
//		//delete a_this;
//		//return nullptr;
//	}
//}