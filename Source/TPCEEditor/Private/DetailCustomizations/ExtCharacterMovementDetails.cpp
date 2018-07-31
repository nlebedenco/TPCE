#include "DetailCustomizations/ExtCharacterMovementDetails.h"
#include "PropertyEditing.h"
#include "IDetailsView.h"
#include "GameFramework/ExtCharacterMovementComponent.h"
#include "SlateBasics.h"
#include "UnrealEd.h"
#include "EditorCategoryUtils.h"
#include "IPropertyUtilities.h"

#define LOCTEXT_NAMESPACE "ExtCharacterMovementDetails" 

DEFINE_LOG_CATEGORY_STATIC(LogExtCharacterMovementDetails, Log, All);


void FExtCharacterMovementDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	static const FText GroundFrictionToolTip = LOCTEXT("GroundFrictionToolTip", "Determines how quickly the character can change direction and optionally brake.\n"
		"This property stores a temporary value calculated for the frame from WalkFriction or WalkFrictionCrouched.\n"
		"@see WalkFriction, WalkFrictionCrouched");

	static const FText MaxAccelerationToolTip = LOCTEXT("MaxAccelerationToolTip", "Max Acceleration (rate of change of velocity)\n"
		"This property stores a temporary value calculated for the frame. Use the corresponding max acceleration property of each specific movement mode\n"
		"to set the desired acceleration.\n"
		"@see MaxWalkAcceleration, MaxWalkAccelerationCrouched, MaxFallingAcceleration, MaxSwimAcceleration, MaxFlyAcceleration");

	TSharedRef<IPropertyHandle> GroundFrictionProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UExtCharacterMovementComponent, GroundFriction), UCharacterMovementComponent::StaticClass());
	GroundFrictionProperty->SetToolTipText(GroundFrictionToolTip);

	TSharedRef<IPropertyHandle> MaxAccelerationProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UExtCharacterMovementComponent, MaxAcceleration), UCharacterMovementComponent::StaticClass());
	MaxAccelerationProperty->SetToolTipText(MaxAccelerationToolTip);

	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UExtCharacterMovementComponent, MaxWalkSpeedCrouched), UCharacterMovementComponent::StaticClass());

	IDetailCategoryBuilder& RagdollCategory = DetailBuilder.EditCategory(TEXT("Character Movement: Ragdoll"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& PivotTurnCategory = DetailBuilder.EditCategory(TEXT("Character Movement: PivotTurn"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& TurnInPlaceCategory = DetailBuilder.EditCategory(TEXT("Character Movement: TurnInPlace"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
}

TSharedRef<IDetailCustomization> FExtCharacterMovementDetails::MakeInstance()
{
	return MakeShareable(new FExtCharacterMovementDetails);
}

#undef LOCTEXT_NAMESPACE