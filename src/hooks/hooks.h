//#pragma once
//
//namespace KMCCT {
//	bool Install();
//
//	class DialogueItemConstructorCall {
//        SINGLETONHEADER(DialogueItemConstructorCall)
//	public:
//		bool Install();
//
//	private:
//
//		static RE::DialogueItem* Thunk(
//			RE::DialogueItem* a_dialogueItem,
//			RE::TESQuest* a_quest,
//			RE::TESTopic* a_topic,
//			RE::TESTopicInfo* a_topicInfo,
//			RE::TESObjectREFR* a_speaker);
//
//		inline static REL::Relocation<decltype(&Thunk)> _func;
//	};
//}