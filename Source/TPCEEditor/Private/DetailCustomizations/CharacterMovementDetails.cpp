#include "DetailCustomizations/CharacterMovementDetails.h"
#include "PropertyEditing.h"
#include "IDetailsView.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SlateBasics.h"
#include "UnrealEd.h"
#include "EditorCategoryUtils.h"

#define LOCTEXT_NAMESPACE "CharacterMovementDetails" 

DEFINE_LOG_CATEGORY_STATIC(LogCharacterMovementDetails, Log, All);


void FCharacterMovementDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& VelocityCategory = DetailBuilder.EditCategory("Velocity", FText::GetEmpty(), ECategoryPriority::Important);

	IDetailCategoryBuilder& MovementComponentCategory = DetailBuilder.EditCategory(TEXT("MovementComponent"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& PlanarMovementCategory = DetailBuilder.EditCategory(TEXT("PlanarMovement"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& NavMovementCategory = DetailBuilder.EditCategory(TEXT("NavMovement"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& CharacterMovementCategory = DetailBuilder.EditCategory(TEXT("Character Movement"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& GeneralSettingsCategory = DetailBuilder.EditCategory(TEXT("Character Movement (General Settings)"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& RotationSettingsCategory = DetailBuilder.EditCategory(TEXT("Character Movement (Rotation Settings)"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& NetworkingCategory = DetailBuilder.EditCategory(TEXT("Character Movement (Networking)"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& WalkingCategory = DetailBuilder.EditCategory(TEXT("Character Movement: Walking"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& NavMeshMovementCategory = DetailBuilder.EditCategory(TEXT("Character Movement: NavMesh Movement"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& AvoidanceCategory = DetailBuilder.EditCategory(TEXT("Character Movement: Avoidance"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& JumpFallingCategory = DetailBuilder.EditCategory(TEXT("Character Movement: Jumping / Falling"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& SwimmingCategory = DetailBuilder.EditCategory(TEXT("Character Movement: Swimming"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& FlyingCategory = DetailBuilder.EditCategory(TEXT("Character Movement: Flying"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& CustomMovementCategory = DetailBuilder.EditCategory(TEXT("Character Movement: Custom Movement"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& PhysicsInteractionCategory = DetailBuilder.EditCategory(TEXT("Character Movement: Physics Interaction"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);

	IDetailCategoryBuilder& TagsCategory = DetailBuilder.EditCategory(TEXT("Tags"), FText::GetEmpty(), ECategoryPriority::Default);
	IDetailCategoryBuilder& ActivationCategory = DetailBuilder.EditCategory(TEXT("Activation"), FText::GetEmpty(), ECategoryPriority::Default);
	IDetailCategoryBuilder& ComponentReplicationCategory = DetailBuilder.EditCategory(TEXT("ComponentReplication"), FText::GetEmpty(), ECategoryPriority::Default);
	IDetailCategoryBuilder& CookingCategory = DetailBuilder.EditCategory(TEXT("Cooking"), FText::GetEmpty(), ECategoryPriority::Default);
	IDetailCategoryBuilder& EventsCategory = DetailBuilder.EditCategory(TEXT("Events"), FText::GetEmpty(), ECategoryPriority::Default);
	IDetailCategoryBuilder& CollisionCategory = DetailBuilder.EditCategory(TEXT("Collision"), FText::GetEmpty(), ECategoryPriority::Default);
	IDetailCategoryBuilder& RootMotionCategory = DetailBuilder.EditCategory(TEXT("RootMotion"), FText::GetEmpty(), ECategoryPriority::Default);
}

TSharedRef<IDetailCustomization> FCharacterMovementDetails::MakeInstance()
{
	return MakeShareable(new FCharacterMovementDetails);
}

#undef LOCTEXT_NAMESPACE