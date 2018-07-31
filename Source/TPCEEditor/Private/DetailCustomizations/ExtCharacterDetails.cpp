#include "DetailCustomizations/ExtCharacterDetails.h"
#include "PropertyEditing.h"
#include "IDetailsView.h"
#include "GameFramework/ExtCharacter.h"
#include "SlateBasics.h"
#include "UnrealEd.h"
#include "EditorCategoryUtils.h"

#define LOCTEXT_NAMESPACE "ExtCharacterDetails" 

DEFINE_LOG_CATEGORY_STATIC(LogExtCharacterDetails, Log, All);


void FExtCharacterDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& AnimationCategory = DetailBuilder.EditCategory(TEXT("Animation"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
	IDetailCategoryBuilder& RagdollCategory = DetailBuilder.EditCategory(TEXT("Ragdoll"), FText::GetEmpty(), ECategoryPriority::TypeSpecific);
}

TSharedRef<IDetailCustomization> FExtCharacterDetails::MakeInstance()
{
	return MakeShareable(new FExtCharacterDetails);
}

#undef LOCTEXT_NAMESPACE