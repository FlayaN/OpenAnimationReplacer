#pragma once

#include "Conditions.h"
#include "Settings.h"

struct ReplacementAnimationFile
{
	struct Variant
	{
		Variant(std::string_view a_fullPath) :
			fullPath(a_fullPath) {}

		std::string fullPath;
		std::optional<std::string> hash = std::nullopt;
	};

	ReplacementAnimationFile(std::string_view a_fullPath);
	ReplacementAnimationFile(std::string_view a_fullPath, std::vector<Variant>& a_variants);

	std::string GetOriginalPath() const;

	std::string fullPath;
	std::optional<std::string> hash = std::nullopt;
	std::optional<std::vector<Variant>> variants = std::nullopt;
};

enum class CustomBlendType : int
{
	kInterrupt = 0,
	kLoop = 1,
	kEcho = 2
};

class ReplacementAnimation
{
public:
	enum class VariantMode : uint8_t
	{
	    kRandom = 0,
		kSequential = 1
	};

	class Variant
	{
	public:
		Variant(uint16_t a_index, std::string_view a_filename, int32_t a_order) :
			_index(a_index),
			_filename(a_filename),
			_order(a_order)
		{}

		uint16_t GetIndex() const { return _index; }
		std::string_view GetFilename() const { return _filename; }
		bool IsDisabled() const { return _bDisabled; }
		void SetDisabled(bool a_bDisable) { _bDisabled = a_bDisable; }

		// random mode
		float GetWeight() const { return _weight; }
		void SetWeight(float a_weight) { _weight = a_weight; }

		// sequential mode
		int32_t GetOrder() const { return _order;}
		void SetOrder(int32_t a_order) { _order = a_order; }

		bool ShouldPlayOnce() const { return _bPlayOnce; }
		void SetPlayOnce(bool a_bPlayOnce) { _bPlayOnce = a_bPlayOnce; }

		bool ShouldSaveToJson(VariantMode a_variantMode) const;

		void ResetSettings();

	protected:
		uint16_t _index = static_cast<uint16_t>(-1);
		std::string _filename;
		bool _bDisabled = false;

		// random mode
		float _weight = 1.f;

		// sequential mode
		int32_t _order;
		bool _bPlayOnce = false;
	};

	class Variants
	{
	public:
		Variants(std::vector<Variant>& a_variants) :
			_variants(std::move(a_variants))
		{
			UpdateVariantCache();
		}

		uint16_t GetVariantIndex() const;
		uint16_t GetVariantIndex(class ActiveClip* a_activeClip) const;
		void UpdateVariantCache();

		[[nodiscard]] VariantMode GetVariantMode() const { return _variantMode; }
		void SetVariantMode(VariantMode a_variantMode) { _variantMode = a_variantMode; }

		bool CanReplaceOnLoopBeforeSequenceFinishes() const { return _bLetReplaceOnLoopBeforeSequenceEnds; }
		void LetReplaceOnLoopBeforeSequenceFinishes(bool a_bEnable) { _bLetReplaceOnLoopBeforeSequenceEnds = a_bEnable; }

		RE::BSVisit::BSVisitControl ForEachVariant(const std::function<RE::BSVisit::BSVisitControl(Variant&)>& a_func);
		RE::BSVisit::BSVisitControl ForEachVariant(const std::function<RE::BSVisit::BSVisitControl(const Variant&)>& a_func) const;

		void SwapVariants(int32_t a_variantIndexA, int32_t a_variantIndexB);

		size_t GetVariantCount() const { return _variants.size(); }
		size_t GetActiveVariantCount() const;
		Variant* GetActiveVariant(size_t a_variantIndex) const;

	protected:
		friend class ReplacementAnimation;
		ReplacementAnimation* _parentReplacementAnimation = nullptr;
		std::vector<Variant> _variants;

		mutable SharedLock _lock;
		std::vector<Variant*> _activeVariants;

		// modes
		VariantMode _variantMode = VariantMode::kRandom;

		// random mode
		float _totalWeight = 0.f;
		std::vector<float> _cumulativeWeights;

		// sequential mode
		bool _bLetReplaceOnLoopBeforeSequenceEnds = false;
	};

	ReplacementAnimation(uint16_t a_index, uint16_t a_originalIndex, int32_t a_priority, std::string_view a_path, std::string_view a_projectName, Conditions::ConditionSet* a_conditionSet);
	ReplacementAnimation(std::vector<Variant>& a_variants, uint16_t a_originalIndex, int32_t a_priority, std::string_view a_path, std::string_view a_projectName, Conditions::ConditionSet* a_conditionSet);

	bool HasVariants() const { return std::holds_alternative<Variants>(_index); }
	Variants& GetVariants() { return std::get<Variants>(_index); }
	const Variants& GetVariants() const { return std::get<Variants>(_index); }

	bool ShouldSaveToJson() const;

	bool IsDisabled() const { return _bDisabled || _bDisabledByParent; }

	uint16_t GetIndex() const;
	uint16_t GetIndex(ActiveClip* a_activeClip) const;
	uint16_t GetOriginalIndex() const { return _originalIndex; }
	int32_t GetPriority() const { return _priority; }
	bool GetDisabled() const { return _bDisabled; }
	bool GetIgnoreDontConvertAnnotationsToTriggersFlag() const { return _bIgnoreDontConvertAnnotationsToTriggersFlag; }
	bool GetTriggersFromAnnotationsOnly() const { return _bTriggersFromAnnotationsOnly; }
	bool GetInterruptible() const { return _bInterruptible; }
	bool GetReplaceOnLoop() const { return _bReplaceOnLoop; }
	bool GetReplaceOnEcho() const { return _bReplaceOnEcho; }
	bool HasCustomBlendTime(CustomBlendType a_type) const;
	float GetCustomBlendTime(CustomBlendType a_type) const;
	bool GetKeepRandomResultsOnLoop() const { return _bKeepRandomResultsOnLoop; }
	bool GetShareRandomResults() const { return _bShareRandomResults; }
	void SetPriority(int32_t a_priority) { _priority = a_priority; }
	void SetDisabled(bool a_bDisable) { _bDisabled = a_bDisable; }
	void SetDisabledByParent(bool a_bDisable) { _bDisabledByParent = a_bDisable; }
	void SetIgnoreDontConvertAnnotationsToTriggersFlag(bool a_enable) { _bIgnoreDontConvertAnnotationsToTriggersFlag = a_enable; }
	void SetTriggersFromAnnotationsOnly(bool a_enable) { _bTriggersFromAnnotationsOnly = a_enable; }
	void SetInterruptible(bool a_bEnable) { _bInterruptible = a_bEnable; }
	void SetReplaceOnLoop(bool a_bEnable) { _bReplaceOnLoop = a_bEnable; }
	void SetReplaceOnEcho(bool a_bEnable) { _bReplaceOnEcho = a_bEnable; }
	void ToggleCustomBlendTime(CustomBlendType a_type, bool a_bEnable);
	void SetCustomBlendTime(CustomBlendType a_type, float a_value);
	void SetKeepRandomResultsOnLoop(bool a_bEnable) { _bKeepRandomResultsOnLoop = a_bEnable; }
	void SetShareRandomResults(bool a_bEnable) { _bShareRandomResults = a_bEnable; }
	std::string_view GetAnimPath() const { return _path; }
	std::string_view GetProjectName() const { return _projectName; }
	Conditions::ConditionSet* GetConditionSet() const { return _conditionSet; }
	SubMod* GetParentSubMod() const;

	bool IsSynchronizedAnimation() const { return _bSynchronized; }
	void MarkAsSynchronizedAnimation(bool a_bSynchronized);
	void SetSynchronizedConditionSet(Conditions::ConditionSet* a_synchronizedConditionSet);

	void LoadAnimData(const struct ReplacementAnimData& a_replacementAnimData);
	void ResetVariants();
	void UpdateVariantCache();
	std::string_view GetVariantFilename(uint16_t a_variantIndex) const;
	RE::BSVisit::BSVisitControl ForEachVariant(const std::function<RE::BSVisit::BSVisitControl(Variant&)>& a_func);
	RE::BSVisit::BSVisitControl ForEachVariant(const std::function<RE::BSVisit::BSVisitControl(const Variant&)>& a_func) const;

	bool EvaluateConditions(RE::TESObjectREFR* a_refr, RE::hkbClipGenerator* a_clipGenerator) const;
	bool EvaluateSynchronizedConditions(RE::TESObjectREFR* a_sourceRefr, RE::TESObjectREFR* a_targetRefr, RE::hkbClipGenerator* a_clipGenerator) const;

protected:
	std::variant<uint16_t, Variants> _index;
	uint16_t _originalIndex;
	int32_t _priority;
	bool _bDisabled = false;
	bool _bIgnoreDontConvertAnnotationsToTriggersFlag = false;
	bool _bTriggersFromAnnotationsOnly = false;
	bool _bInterruptible = false;
	bool _bCustomBlendTimeOnInterrupt = false;
	float _blendTimeOnInterrupt = Settings::fDefaultBlendTimeOnInterrupt;
	bool _bReplaceOnLoop = true;
	bool _bCustomBlendTimeOnLoop = false;
	float _blendTimeOnLoop = Settings::fDefaultBlendTimeOnLoop;
	bool _bReplaceOnEcho = false;
	bool _bCustomBlendTimeOnEcho = false;
	float _blendTimeOnEcho = Settings::fDefaultBlendTimeOnEcho;
	bool _bKeepRandomResultsOnLoop = false;
	bool _bShareRandomResults = false;
	std::string _path;
	std::string _projectName;
	Conditions::ConditionSet* _conditionSet;
	Conditions::ConditionSet* _synchronizedConditionSet = nullptr;

	bool _bSynchronized = false;

	friend class SubMod;
	SubMod* _parentSubMod = nullptr;

	bool _bDisabledByParent = false;
};
