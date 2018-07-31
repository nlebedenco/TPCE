#include "DetailCustomizations/CharacterDetails.h"
#include "PropertyEditing.h"
#include "IDetailsView.h"
#include "GameFramework/Character.h"
#include "SlateBasics.h"
#include "UnrealEd.h"
#include "EditorCategoryUtils.h"

#define LOCTEXT_NAMESPACE "CharacterDetails" 

DEFINE_LOG_CATEGORY_STATIC(LogCharacterDetails, Log, All);


void FCharacterDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& DebugCategory = DetailBuilder.EditCategory(TEXT("Debug"), FText::GetEmpty(), ECategoryPriority::Variable);

	IDetailCategoryBuilder& ActorCategory = DetailBuilder.EditCategory(TEXT("Actor"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& PawnCategory = DetailBuilder.EditCategory(TEXT("Pawn"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& CharacterCategory = DetailBuilder.EditCategory(TEXT("Character"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& InputCategory = DetailBuilder.EditCategory(TEXT("Input"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& CameraCategory = DetailBuilder.EditCategory(TEXT("Camera"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	
	IDetailCategoryBuilder& ReplicationCategory = DetailBuilder.EditCategory(TEXT("Replication"), FText::GetEmpty(), ECategoryPriority::Default);
	IDetailCategoryBuilder& RenderingCategory = DetailBuilder.EditCategory(TEXT("Rendering"), FText::GetEmpty(), ECategoryPriority::Default);
	IDetailCategoryBuilder& LODCategory = DetailBuilder.EditCategory(TEXT("LOD"), FText::GetEmpty(), ECategoryPriority::Default);
}

TSharedRef<IDetailCustomization> FCharacterDetails::MakeInstance()
{
	return MakeShareable(new FCharacterDetails);
}

#undef LOCTEXT_NAMESPACE