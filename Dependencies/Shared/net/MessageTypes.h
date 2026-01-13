/**
 * @file MessageTypes.h
 * @brief Organized message type enumerations for client-server protocol
 *
 * This file organizes the legacy message IDs into strongly-typed enums
 * for better type safety and code readability.
 *
 * IMPORTANT: When modifying message IDs, increment PROTOCOL_VERSION
 * and ensure both client and server are updated simultaneously.
 */

#pragma once

#include "../Types.h"

namespace Helbreath::Net {

// =============================================================================
// Protocol Version - Increment when message format changes
// =============================================================================

constexpr u16 PROTOCOL_VERSION_MAJOR = 3;
constexpr u16 PROTOCOL_VERSION_MINOR = 82;
constexpr u32 PROTOCOL_VERSION = (PROTOCOL_VERSION_MAJOR << 16) | PROTOCOL_VERSION_MINOR;

// =============================================================================
// Message Categories
// =============================================================================

enum class MessageCategory : u8 {
    System      = 0x00,  // System/connection messages
    Login       = 0x01,  // Login/authentication
    Character   = 0x02,  // Character management
    Motion      = 0x03,  // Movement and actions
    Combat      = 0x04,  // Combat related
    Item        = 0x05,  // Item operations
    Magic       = 0x06,  // Magic/skills
    Chat        = 0x07,  // Chat messages
    Guild       = 0x08,  // Guild operations
    Party       = 0x09,  // Party operations
    Quest       = 0x0A,  // Quest operations
    Notify      = 0x0B,  // Server notifications
    Crusade     = 0x0C,  // Crusade/war events
    Admin       = 0x0D,  // Admin commands
    Config      = 0x0E,  // Configuration data
};

// =============================================================================
// Core Message IDs (from NetMessages.h)
// =============================================================================

namespace MsgId {
    // Initialization
    constexpr u32 RequestInitPlayer  = 0x05040205;
    constexpr u32 ResponseInitPlayer = 0x05040206;
    constexpr u32 RequestInitData    = 0x05080404;
    constexpr u32 ResponseInitData   = 0x05080405;

    // Motion
    constexpr u32 CommandMotion   = 0x0FA314D5;
    constexpr u32 ResponseMotion  = 0x0FA314D6;
    constexpr u32 EventMotion     = 0x0FA314D7;
    constexpr u32 EventLog        = 0x0FA314D8;
    constexpr u32 EventCommon     = 0x0FA314DB;
    constexpr u32 CommandCommon   = 0x0FA314DC;

    // Login
    constexpr u32 RequestLogin            = 0x0FC94201;
    constexpr u32 RequestCreateAccount    = 0x0FC94202;
    constexpr u32 ResponseLog             = 0x0FC94203;
    constexpr u32 RequestCreateCharacter  = 0x0FC94204;
    constexpr u32 RequestEnterGame        = 0x0FC94205;
    constexpr u32 ResponseEnterGame       = 0x0FC94206;
    constexpr u32 RequestDeleteCharacter  = 0x0FC94207;

    // Guild
    constexpr u32 RequestCreateGuild   = 0x0FC94208;
    constexpr u32 ResponseCreateGuild  = 0x0FC94209;
    constexpr u32 RequestDisbandGuild  = 0x0FC9420A;
    constexpr u32 ResponseDisbandGuild = 0x0FC9420B;

    // Notify
    constexpr u32 Notify = 0x0FA314D0;

    // Chat
    constexpr u32 CheckConnection = 0x03203203;
    constexpr u32 ChatMsg         = 0x03203204;

    // Player Data
    constexpr u32 RequestPlayerData      = 0x0C152210;
    constexpr u32 ResponsePlayerData     = 0x0C152211;
    constexpr u32 RequestSavePlayerData  = 0x0DF3076F;
    constexpr u32 ResponseSavePlayerData = 0x0C152212;

    // Teleport
    constexpr u32 RequestTeleport         = 0x0EA03201;
    constexpr u32 RequestCityHallTeleport = 0x0EA03202;
    constexpr u32 RequestHeldenianTeleport = 0x0EA03206;

    // Configuration
    constexpr u32 ItemConfig    = 0x0FA314D9;
    constexpr u32 NpcConfig     = 0x0FA314DA;
    constexpr u32 MagicConfig   = 0x0FA314DB;
    constexpr u32 SkillConfig   = 0x0FA314DC;
    constexpr u32 PlayerItems   = 0x0FA314DD;
    constexpr u32 PotionConfig  = 0x0FA314DE;
    constexpr u32 PlayerChar    = 0x0FA40000;
    constexpr u32 QuestConfig   = 0x0FA40001;
    constexpr u32 BuildItemConfig = 0x0FA40002;

    // Party
    constexpr u32 PartyOperation = 0x3C00123A;

    // Crusade
    constexpr u32 CollectedMana  = 0x3AE90000;
    constexpr u32 MeteorStrike   = 0x3AE90001;
}

// =============================================================================
// Common Type IDs (DEF_COMMONTYPE_*)
// =============================================================================

namespace CommonType {
    constexpr u16 ItemDrop            = 0x0A01;
    constexpr u16 EquipItem           = 0x0A02;
    constexpr u16 ReqListContents     = 0x0A03;
    constexpr u16 ReqPurchaseItem     = 0x0A04;
    constexpr u16 GiveItemToChar      = 0x0A05;
    constexpr u16 JoinGuildApprove    = 0x0A06;
    constexpr u16 JoinGuildReject     = 0x0A07;
    constexpr u16 DismissGuildApprove = 0x0A08;
    constexpr u16 DismissGuildReject  = 0x0A09;
    constexpr u16 ReleaseItem         = 0x0A0A;
    constexpr u16 ToggleCombatMode    = 0x0A0B;
    constexpr u16 SetItem             = 0x0A0C;
    constexpr u16 Magic               = 0x0A0D;
    constexpr u16 ReqStudyMagic       = 0x0A0E;
    constexpr u16 ReqTrainSkill       = 0x0A0F;
    constexpr u16 ReqGetRewardMoney   = 0x0A10;
    constexpr u16 ReqUseItem          = 0x0A11;
    constexpr u16 ReqUseSkill         = 0x0A12;
    constexpr u16 ReqSellItem         = 0x0A13;
    constexpr u16 ReqRepairItem       = 0x0A14;
    constexpr u16 ReqSellItemConfirm  = 0x0A15;
    constexpr u16 ReqRepairItemConfirm = 0x0A16;
    constexpr u16 ToggleSafeAttackMode = 0x0A18;
    constexpr u16 ReqCreatePotion     = 0x0A19;
    constexpr u16 TalkToNpc           = 0x0A1A;
    constexpr u16 ExchangeItemToChar  = 0x0A1E;
    constexpr u16 SetExchangeItem     = 0x0A1F;
    constexpr u16 ConfirmExchangeItem = 0x0A20;
    constexpr u16 CancelExchangeItem  = 0x0A21;
    constexpr u16 QuestAccepted       = 0x0A22;
    constexpr u16 BuildItem           = 0x0A23;
    constexpr u16 GetMagicAbility     = 0x0A24;
    constexpr u16 UpgradeItem         = 0x0A58;
    constexpr u16 ReqGuildName        = 0x0A59;
    constexpr u16 ChangePlayMode      = 0x0A60;
    constexpr u16 ReqCreateSlate      = 0x0A61;
    constexpr u16 CraftItem           = 0x0A28;
    constexpr u16 EnchantItem         = 0x0A73;
    constexpr u16 UpgradeEnchant      = 0x0A74;
    constexpr u16 DisenchantItem      = 0x0A75;

    // Party
    constexpr u16 RequestAcceptJoinParty = 0x0A30;
    constexpr u16 RequestJoinParty       = 0x0A31;
    constexpr u16 ResponseJoinParty      = 0x0A32;

    // Special Abilities
    constexpr u16 RequestActivateSpecAbility = 0x0A40;
    constexpr u16 RequestCancelQuest         = 0x0A50;
    constexpr u16 RequestSelectCrusadeDuty   = 0x0A51;
    constexpr u16 RequestMapStatus           = 0x0A52;
    constexpr u16 RequestHelp                = 0x0A53;

    // Guild Teleport
    constexpr u16 SetGuildTeleportLoc  = 0x0A54;
    constexpr u16 GuildTeleport        = 0x0A55;
    constexpr u16 SummonWarUnit        = 0x0A56;
    constexpr u16 SetGuildConstructLoc = 0x0A57;

    // HP Bar
    constexpr u16 ReqGetNpcHp = 0x1F12;
    constexpr u16 SendNpcHp   = 0x1F13;

    // Repair All
    constexpr u16 ReqRepairAll        = 0x0F10;
    constexpr u16 NotifyRepairAllPrices = 0x0F11;
    constexpr u16 ReqRepairAllDelete  = 0x0F12;
    constexpr u16 ReqRepairAllConfirm = 0x0F13;
}

// =============================================================================
// Notification Type IDs (DEF_NOTIFY_*)
// =============================================================================

namespace NotifyType {
    constexpr u16 ItemObtained          = 0x0B01;
    constexpr u16 QueryJoinGuildPerm    = 0x0B02;
    constexpr u16 QueryDismissGuildPerm = 0x0B03;
    constexpr u16 WaitForGuildOp        = 0x0B04;
    constexpr u16 CannotCarryMoreItem   = 0x0B05;
    constexpr u16 ItemPurchased         = 0x0B06;
    constexpr u16 HP                    = 0x0B07;
    constexpr u16 NotEnoughGold         = 0x0B08;
    constexpr u16 Killed                = 0x0B09;
    constexpr u16 EXP                   = 0x0B0A;
    constexpr u16 GuildDisbanded        = 0x0B0B;
    constexpr u16 EventMsgString        = 0x0B0C;
    constexpr u16 MagicStudySuccess     = 0x0B10;
    constexpr u16 MagicStudyFail        = 0x0B11;
    constexpr u16 SkillTrainSuccess     = 0x0B12;
    constexpr u16 SkillTrainFail        = 0x0B13;
    constexpr u16 MP                    = 0x0B14;
    constexpr u16 SP                    = 0x0B15;
    constexpr u16 LevelUp               = 0x0B16;
    constexpr u16 ItemLifespanEnd       = 0x0B17;
    constexpr u16 LimitedLevel          = 0x0B18;
    constexpr u16 ItemToBank            = 0x0B19;
    constexpr u16 PKPenalty             = 0x0B1A;
    constexpr u16 PKCaptured            = 0x0B1B;
    constexpr u16 EnemyKillReward       = 0x0B1C;
    constexpr u16 NewDynamicObject      = 0x0B21;
    constexpr u16 DelDynamicObject      = 0x0B22;
    constexpr u16 Skill                 = 0x0B23;
    constexpr u16 ServerChange          = 0x0B24;
    constexpr u16 SetItemCount          = 0x0B25;
    constexpr u16 MagicEffectOn         = 0x0B27;
    constexpr u16 MagicEffectOff        = 0x0B28;
    constexpr u16 TotalUsers            = 0x0B29;
    constexpr u16 ShowMap               = 0x0B2B;
    constexpr u16 SellItemPrice         = 0x0B2D;
    constexpr u16 RepairItemPrice       = 0x0B2F;
    constexpr u16 ItemRepaired          = 0x0B30;
    constexpr u16 ItemSold              = 0x0B31;
    constexpr u16 Charisma              = 0x0B32;
    constexpr u16 PlayerOnGame          = 0x0B33;
    constexpr u16 PlayerNotOnGame       = 0x0B34;
    constexpr u16 WhisperModeOn         = 0x0B35;
    constexpr u16 WhisperModeOff        = 0x0B36;
    constexpr u16 PlayerProfile         = 0x0B37;
    constexpr u16 Hunger                = 0x0B39;
    constexpr u16 ToBeRecalled          = 0x0B40;
    constexpr u16 TimeChange            = 0x0B41;
    constexpr u16 PlayerShutup          = 0x0B42;
    constexpr u16 NoticeMsg             = 0x0B46;
    constexpr u16 WeatherChange         = 0x0B4D;
    constexpr u16 ServerShutdown        = 0x0B4E;
    constexpr u16 RewardGold            = 0x0B4F;
    constexpr u16 SafeAttackMode        = 0x0B51;
    constexpr u16 SuperAttackLeft       = 0x0B52;
    constexpr u16 PotionSuccess         = 0x0B56;
    constexpr u16 PotionFail            = 0x0B55;
    constexpr u16 NpcTalk               = 0x0B57;
    constexpr u16 EnemyKills            = 0x0B5A;
    constexpr u16 OpenExchangeWindow    = 0x0B5E;
    constexpr u16 SetExchangeItem       = 0x0B5F;
    constexpr u16 CancelExchangeItem    = 0x0B60;
    constexpr u16 ExchangeComplete      = 0x0B61;
    constexpr u16 QuestContents         = 0x0B66;
    constexpr u16 QuestAborted          = 0x0B67;
    constexpr u16 QuestCompleted        = 0x0B68;
    constexpr u16 QuestReward           = 0x0B69;
    constexpr u16 BuildItemSuccess      = 0x0B70;
    constexpr u16 BuildItemFail         = 0x0B71;
    constexpr u16 ObserverMode          = 0x0B72;
    constexpr u16 GlobalAttackMode      = 0x0B73;
    constexpr u16 DamageMove            = 0x0B74;
    constexpr u16 ForceDisconn          = 0x0B75;
    constexpr u16 FightZoneReserve      = 0x0B76;
    constexpr u16 ResponseCreateParty   = 0x0B80;
    constexpr u16 QueryJoinParty        = 0x0B81;
    constexpr u16 EnergySphereCreated   = 0x0B90;
    constexpr u16 EnergySphereGoalIn    = 0x0B91;
    constexpr u16 SpecialAbilityEnabled = 0x0B92;
    constexpr u16 SpecialAbilityStatus  = 0x0B93;
    constexpr u16 Crusade               = 0x0B94;
    constexpr u16 LockedMap             = 0x0B95;
    constexpr u16 DutySelected          = 0x0B96;
    constexpr u16 MeteorStrikeComing    = 0x0B9B;
    constexpr u16 MeteorStrikeHit       = 0x0B9C;
    constexpr u16 GrandMagicResult      = 0x0B9D;
    constexpr u16 ConstructionPoint     = 0x0B9F;
    constexpr u16 Party                 = 0x0BA2;
    constexpr u16 ItemAttributeChange   = 0x0BA3;
    constexpr u16 ItemUpgradeFail       = 0x0BA8;
    constexpr u16 SpawnEvent            = 0x0BAA;
    constexpr u16 CraftingSuccess       = 0x0BF0;
    constexpr u16 CraftingFail          = 0x0BF1;
    constexpr u16 HeldenianStart        = 0x0BEA;
    constexpr u16 HeldenianCount        = 0x0BEC;
    constexpr u16 HeldenianEnd          = 0x0BE7;
    constexpr u16 HeldenianTeleport     = 0x0BE6;
    constexpr u16 QuestCounter          = 0x0BE2;
    constexpr u16 MonsterCount          = 0x0BE3;
    constexpr u16 MobKills              = 0x0A68;
    constexpr u16 ChangePlayMode        = 0x0BA9;
}

// =============================================================================
// Login Response Types (DEF_LOGRESMSGTYPE_*)
// =============================================================================

namespace LoginResponse {
    constexpr u16 Confirm                 = 0x0F14;
    constexpr u16 Reject                  = 0x0F15;
    constexpr u16 PasswordMismatch        = 0x0F16;
    constexpr u16 NotExistingAccount      = 0x0F17;
    constexpr u16 NewAccountCreated       = 0x0F18;
    constexpr u16 NewAccountFailed        = 0x0F19;
    constexpr u16 AlreadyExistingAccount  = 0x0F1A;
    constexpr u16 NotExistingCharacter    = 0x0F1B;
    constexpr u16 NewCharacterCreated     = 0x0F1C;
    constexpr u16 NewCharacterFailed      = 0x0F1D;
    constexpr u16 AlreadyExistingChar     = 0x0F1E;
    constexpr u16 CharacterDeleted        = 0x0F1F;
    constexpr u16 PasswordChangeSuccess   = 0x0A00;
    constexpr u16 PasswordChangeFail      = 0x0A01;
}

// =============================================================================
// Enter Game Response Types
// =============================================================================

namespace EnterGameResponse {
    constexpr u16 Playing        = 0x0F20;
    constexpr u16 Reject         = 0x0F21;
    constexpr u16 Confirm        = 0x0F22;
    constexpr u16 ForceDisconn   = 0x0F23;
}

// =============================================================================
// Gate Server Messages (GSM_*)
// =============================================================================

namespace GateServer {
    constexpr u8 RequestFindCharacter  = 0x01;
    constexpr u8 ResponseFindCharacter = 0x02;
    constexpr u8 GrandMagicResult      = 0x03;
    constexpr u8 GrandMagicLaunch      = 0x04;
    constexpr u8 CollectedMana         = 0x05;
    constexpr u8 BeginCrusade          = 0x06;
    constexpr u8 EndCrusade            = 0x07;
    constexpr u8 MiddleMapStatus       = 0x08;
    constexpr u8 SetGuildTeleportLoc   = 0x09;
    constexpr u8 ConstructionPoint     = 0x0A;
    constexpr u8 SetGuildConstructLoc  = 0x0B;
    constexpr u8 ChatMsg               = 0x0C;
    constexpr u8 WhisperMsg            = 0x0D;
    constexpr u8 Disconnect            = 0x0E;
    constexpr u8 RequestSummonPlayer   = 0x0F;
    constexpr u8 RequestShutupPlayer   = 0x10;
    constexpr u8 ResponseShutupPlayer  = 0x11;
    constexpr u8 BeginApocalypse       = 0x13;
    constexpr u8 EndApocalypse         = 0x14;
    constexpr u8 RequestSummonGuild    = 0x15;
    constexpr u8 RequestSummonAll      = 0x16;
    constexpr u8 EndHeldenian          = 0x17;
    constexpr u8 UpdateConfigs         = 0x18;
    constexpr u8 StartHeldenian        = 0x19;
}

} // namespace Helbreath::Net
