// Copyright Erik Shukhyan


#include "AbilitySystem/AuraAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

UAuraAttributeSet::UAuraAttributeSet()
{
	InitHealth(50.f);
	InitMaxHealth(100.f);
	InitMana(10.f);
	InitMaxMana(50.f);
}

void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
}

void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

void UAuraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}
}

void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth);
}

void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, OldMaxHealth);
}

void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, OldMana);
}

void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, OldMaxMana);
}

void UAuraAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& EffectPropertiesOut) const
{
	// Source = Causer of the effect, Target = target of the effect (owner of this AS)
	
	EffectPropertiesOut.EffectContextHandle = Data.EffectSpec.GetContext();
	
	EffectPropertiesOut.SourceASC = EffectPropertiesOut.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	if (IsValid(EffectPropertiesOut.SourceASC) && EffectPropertiesOut.SourceASC->AbilityActorInfo.IsValid() && EffectPropertiesOut.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		EffectPropertiesOut.SourceAvatarActor = EffectPropertiesOut.SourceASC->GetAvatarActor();
		EffectPropertiesOut.SourceController = EffectPropertiesOut.SourceASC->AbilityActorInfo->PlayerController.Get();
		if (!IsValid(EffectPropertiesOut.SourceController) && IsValid(EffectPropertiesOut.SourceAvatarActor))
		{
			if (const APawn* Pawn = Cast<APawn>(EffectPropertiesOut.SourceAvatarActor))
			{
				EffectPropertiesOut.SourceController = Pawn->GetController();
			}
		}
		if (IsValid(EffectPropertiesOut.SourceController))
		{
			//EffectPropertiesOut.SourceCharacter = SourceController->GetCharacter();
			EffectPropertiesOut.SourceCharacter = Cast<ACharacter>(EffectPropertiesOut.SourceController->GetPawn());
		}
	}

	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		EffectPropertiesOut.TargetAvatarActor = Data.Target.GetAvatarActor();
		EffectPropertiesOut.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		EffectPropertiesOut.TargetCharacter = Cast<ACharacter>(EffectPropertiesOut.TargetAvatarActor);
		EffectPropertiesOut.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(EffectPropertiesOut.TargetAvatarActor);
	}
}