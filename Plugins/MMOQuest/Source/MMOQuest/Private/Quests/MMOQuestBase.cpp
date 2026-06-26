#include "Quests/MMOQuestBase.h"
#include "Quests/MMOQuestComponent.h"
#include "Engine/World.h"

bool UMMOQuestBase::IsRepeatableNow(const UMMOQuestComponent* Component) const
{
	if (!bRepeatable || !Component)
	{
		return false;
	}
	return !Component->IsQuestOnCooldown(this);
}

bool UMMOQuestBase::MeetsPrerequisites(const UMMOQuestComponent* Component) const
{
	if (!Component)
	{
		return false;
	}
	for (const FGameplayTag& ReqID : RequiredQuestIDs)
	{
		if (!Component->HasCompletedQuest(ReqID))
		{
			return false;
		}
	}
	return true;
}

FText UMMOQuestBase::GetQuestObjectiveDisplay(int32 ObjectiveIndex) const
{
	if (ObjectiveIndex < 0 || ObjectiveIndex >= ObjectiveTexts.Num())
	{
		return FText::GetEmpty();
	}
	return ObjectiveTexts[ObjectiveIndex];
}

#if WITH_EDITOR
void UMMOQuestBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (ObjectiveTexts.Num() != Objectives.Num())
	{
		ObjectiveTexts.SetNum(Objectives.Num());
	}
}
#endif
