#ifndef __NPOPUP_ENUMS_H__
#define __NPOPUP_ENUMS_H__

#pragma once

enum class EPopupEvent : mu_uint32
{
	eAcceptButton = 0,
	eCancelButton = 1,
};

enum class EPopupType : mu_uint16
{
	eMessageOnly,
	eAccept,
	eAcceptCancel,
	eDestroyItem,
	eEnhanceItem,
	eDestroyMount,
	eEnhanceMount,
	eDestroyPet,
	eEnhancePet,
	eMax,
};
constexpr mu_uint32 EPopupTypeMax = static_cast<mu_uint32>(EPopupType::eMax);

enum class EPopupPriority : mu_uint32
{
	ePopupCritical, // e.g. Disconnected
	ePopupHigh,
	ePopupMedium,
	ePopupLow,
	eMax,
};
constexpr mu_uint32 EPopupPriorityMax = static_cast<mu_uint32>(EPopupPriority::eMax);

enum class EPopupID : mu_uint16
{
	// OpenID Connect
	eRetrieveOpenIdConfiguration,
	eParseOpenIdConfigurationFailed,
	eRetrieveOpenIdConfigurationFailed,
	eProcessingAuthResponse,
	eInvalidAuthResponse,
	eParseAuthorizationCodeGrantFailed,
	eRetrieveAuthorizationCodeGrantFailed,
	
	// Servers List
	eRetrieveServersList,

	// Network
	eConnecting,
	eFailedConnect,
	eDisconnected,

	eServerRefused,
	eServerBanned,
	eServerFull,

	// Update System
	eUpdateDownloadAlert,
	eUpdateServerListFailed,
	eUpdateFilesListFailed,
	eUpdateFileDownloadFailed,
	eUpdateFileCorrupted,
	eUpdateNoMemory,
	eUpdateDecompressFailed,
	eUpdateStorageFull,
	eUpdateNoServerAvailable,

	// Settings
	eRequireRestart,

	// Login
	eLoggingIn,
	eSocialLoggingIn,
	eLoginFailed,
	eAlreadyConnected,
	eWrongVersion,
	eUsernameWrongLength,
	eUsernameProhibited,
	ePasswordWrongLength,
	ePasswordProhibited,

	// Character Scene
	eWaitingCharacters,
	eLoadingCharacter,

	// Create Character
	eCharacterNameWrongLength,
	eCharacterNameProhibited,
	eCharacterNameAlreadyExists,
	eCharacterCreateNoFreeSlot,
	eCreatingCharacter,

	// Delete Character
	eDeleteCharacter,
	eDeleteCharacterGuildError,
	eDeleteCharacterFailed,
	eDeleteCharacterSucceed,
	eDeletingCharacter,

	// Item
	eDestroyItem,
	eEnhanceItem,

	eDestroyMount,
	eEnhanceMount,

	eDestroyPet,
	eEnhancePet,

	ePartyKick,
	ePartyLeave,

	eSystemError,
	eUnexpectedError,

	eMax,
};

#endif