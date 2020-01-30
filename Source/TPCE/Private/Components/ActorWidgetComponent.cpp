// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorWidgetComponent.h"
#include "Blueprint/ActorWidget.h"
#include "Logging/LogMacros.h"
#include "Kismet/Kismet.h"

DEFINE_LOG_CATEGORY_STATIC(LogActorWidgetComponent, Log, All);

UActorWidgetComponent::UActorWidgetComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = false;
	bHiddenInGame = true;

	// Set common defaults when using widgets on Actors
	SetDrawAtDesiredSize(true);
	SetWidgetSpace(EWidgetSpace::Screen);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

#if WITH_EDITOR

bool UActorWidgetComponent::CanEditChange(const UProperty* InProperty) const
{
	bool bCanChange = Super::CanEditChange(InProperty);

	if (bCanChange)
	{
		const FName PropertyName = (InProperty != NULL) ? InProperty->GetFName() : NAME_None;
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bHiddenInGame)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bAutoActivate))
		{
			bCanChange = false;
		}
	}

	return bCanChange;
}

void UActorWidgetComponent::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	const FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bHiddenInGame))
	{
		if (UWorld* World = GetWorld())
			if (World->IsGameWorld())
				OnHiddenInGameChanged();
	}
	else if (PropertyName == GetVisiblePropertyName())
	{
		if (UWorld* World = GetWorld())
			if (World->IsGameWorld())
				OnVisibilityChanged();
	}

	// Note: Any changes must be made before UActorComponent::PostEditChangeChainProperty is called because components will be reset when UActorComponent reruns construction scripts 
	Super::PostEditChangeProperty(e);
}
#endif


void UActorWidgetComponent::OnHiddenInGameChanged()
{
	Super::OnHiddenInGameChanged();

	// Make sure the widget will not respond to visibility traces
	GetUserWidgetObject()->SetVisibility(IsVisible() ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UActorWidgetComponent::OnVisibilityChanged()
{
	Super::OnVisibilityChanged();

	// Make sure the widget will not respond to visibility traces
	GetUserWidgetObject()->SetVisibility(IsVisible() ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UActorWidgetComponent::InitWidget()
{
	Super::InitWidget();

	if (UActorWidget* ActorWidget = Cast<UActorWidget>(Widget))
		ActorWidget->SetWidgetComponent(this);
}

void UActorWidgetComponent::SetWidget(UUserWidget* InWidget)
{
	// Reset old widget if it exists
	if (UActorWidget* ActorWidget = Cast<UActorWidget>(Widget))
		ActorWidget->SetWidgetComponent(nullptr);

	Super::SetWidget(InWidget);

	// Setup new widget if it exists
	if (UActorWidget* ActorWidget = Cast<UActorWidget>(Widget))
		ActorWidget->SetWidgetComponent(this);
}

void UActorWidgetComponent::OnRegister()
{
	Super::OnRegister();

	if (UWorld* ThisWorld = GetWorld())
	{
		if (ThisWorld->IsGameWorld())
		{
			if (UGameViewportClient* ViewportClient = ThisWorld->GetGameViewport())
			{
				ViewportClient->OnPlayerAdded().AddUObject(this, &ThisClass::OnPlayerAdded);
				ViewportClient->OnPlayerRemoved().AddUObject(this, &ThisClass::OnPlayerRemoved);
			}

			OnAutoAssignPlayerChanged();
		}
	}
}

void UActorWidgetComponent::OnPlayerAdded(int32 PlayerIndex)
{
	ULocalPlayer* Player = GetWorld()->GetGameInstance()->GetLocalPlayerByIndex(PlayerIndex);
	if (PlayerIndex == (AutoAssignPlayer - 1))
	{
		SetOwnerPlayer(Player);
		SetHiddenInGame(false);
		SetActive(true);
	}
}

void UActorWidgetComponent::OnPlayerRemoved(int32 PlayerIndex)
{
	if (PlayerIndex == (int32(AutoAssignPlayer) - 1))
	{
		SetOwnerPlayer(nullptr);
		SetHiddenInGame(true);
		SetActive(false);
	}
}


void UActorWidgetComponent::SetAutoAssignPlayer(EAutoReceiveInput::Type PlayerIndex)
{
	if (AutoAssignPlayer != PlayerIndex)
	{
		AutoAssignPlayer = PlayerIndex;
		OnAutoAssignPlayerChanged();
	}
}

void UActorWidgetComponent::OnAutoAssignPlayerChanged()
{
	if (AutoAssignPlayer > EAutoReceiveInput::Disabled)
	{
		const TArray<ULocalPlayer*>& LocalPlayers = GetWorld()->GetGameInstance()->GetLocalPlayers();
		for (int32 i = 0; i < LocalPlayers.Num(); ++i)
		{
			if (i == (AutoAssignPlayer - 1))
			{
				SetOwnerPlayer(LocalPlayers[i]);
				SetHiddenInGame(false);
				SetActive(true);
				return;
			}
		}
	}

	SetOwnerPlayer(nullptr);
	SetHiddenInGame(true);
	SetActive(false);
}