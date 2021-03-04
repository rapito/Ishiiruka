// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <SlippiGame.h>

#include "Common/CommonTypes.h"
#include "Common/FileUtil.h"
#include "Core/HW/EXI_Device.h"
#include "Core/Slippi/SlippiGameFileLoader.h"
#include "Core/Slippi/SlippiGameReporter.h"
#include "Core/Slippi/SlippiMatchmaking.h"
#include "Core/Slippi/SlippiNetplay.h"
#include "Core/Slippi/SlippiReplayComm.h"
#include "Core/Slippi/SlippiSavestate.h"
#include "Core/Slippi/SlippiSpectate.h"
#include "Core/Slippi/SlippiUser.h"

#define ROLLBACK_MAX_FRAMES 7
#define MAX_NAME_LENGTH 15
#define CONNECT_CODE_LENGTH 8

extern bool g_needInputForFrame;

// Emulated Slippi device used to receive and respond to in-game messages
class CEXISlippi : public IEXIDevice
{
  public:
	CEXISlippi();
	virtual ~CEXISlippi();

	void DMAWrite(u32 _uAddr, u32 _uSize) override;
	void DMARead(u32 addr, u32 size) override;

	bool IsPresent() const override;

  private:
	enum
	{
		CMD_UNKNOWN = 0x0,

		// Recording
		CMD_RECEIVE_COMMANDS = 0x35,
		CMD_RECEIVE_GAME_INFO = 0x36,
		CMD_RECEIVE_POST_FRAME_UPDATE = 0x38,
		CMD_RECEIVE_GAME_END = 0x39,
		CMD_FRAME_BOOKEND = 0x3C,
		CMD_MENU_FRAME = 0x3E,

		// Playback
		CMD_PREPARE_REPLAY = 0x75,
		CMD_READ_FRAME = 0x76,
		CMD_GET_LOCATION = 0x77,
		CMD_IS_FILE_READY = 0x88,
		CMD_IS_STOCK_STEAL = 0x89,
		CMD_GET_GECKO_CODES = 0x8A,

		// Online
		CMD_ONLINE_INPUTS = 0xB0,
		CMD_CAPTURE_SAVESTATE = 0xB1,
		CMD_LOAD_SAVESTATE = 0xB2,
		CMD_GET_MATCH_STATE = 0xB3,
		CMD_FIND_OPPONENT = 0xB4,
		CMD_SET_MATCH_SELECTIONS = 0xB5,
		CMD_OPEN_LOGIN = 0xB6,
		CMD_LOGOUT = 0xB7,
		CMD_UPDATE = 0xB8,
		CMD_GET_ONLINE_STATUS = 0xB9,
		CMD_CLEANUP_CONNECTION = 0xBA,
		CMD_SEND_CHAT_MESSAGE = 0xBB,
		CMD_GET_NEW_SEED = 0xBC,
		CMD_REPORT_GAME = 0xBD,
		CMD_SET_MATCH_INFO = 0xBE,

		// Misc
		CMD_LOG_MESSAGE = 0xD0,
		CMD_FILE_LENGTH = 0xD1,
		CMD_FILE_LOAD = 0xD2,
		CMD_GCT_LENGTH = 0xD3,
		CMD_GCT_LOAD = 0xD4,
	};

	enum
	{
		FRAME_RESP_WAIT = 0,
		FRAME_RESP_CONTINUE = 1,
		FRAME_RESP_TERMINATE = 2,
		FRAME_RESP_FASTFORWARD = 3,
	};

	// This match block is a VS match with P1 Red Falco vs P2 Red Bowser on Battlefield . The proper values will
	// be overwritten
	std::vector<u8> defaultMatchBlock = {
	    0x32, 0x01, 0x86, 0x4C, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x6E, 0x00, 0x1F, 0x00, 0x00,
	    0x01, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
	    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x3F, 0x80,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x78, 0x00,
	    0xC0, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x3F, 0x80,
	    0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x05, 0x00, 0x04, 0x01, 0x00, 0x01, 0x00, 0x00, 0x09, 0x00, 0x78, 0x00,
	    0xC0, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x3F, 0x80,
	    0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x15, 0x03, 0x04, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x09, 0x00, 0x78, 0x00,
	    0xC0, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x3F, 0x80,
	    0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x15, 0x03, 0x04, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x09, 0x00, 0x78, 0x00,
	    0xC0, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x3F, 0x80,
	    0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x21, 0x03, 0x04, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x09, 0x00, 0x78, 0x00,
	    0x40, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x3F, 0x80,
	    0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x21, 0x03, 0x04, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x09, 0x00, 0x78, 0x00,
	    0x40, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x3F, 0x80,
	    0x00, 0x00, 0x3F, 0x80, 0x00, 0x00,
	};

	std::vector<u8> defaultRules = {
	    0x00, 0x34, 0x01, 0x01, // Custom Rules 1
	    0x04, 0x00, 0x0A, 0x00, // Custom Rules 2
	    0x08, 0x01, 0x00, 0x00, // Additional Rules 1
	    0x00, 0x00, 0x08, 0x08, // Additional Rules 2

	    0xFF, 0x00, 0x00, 0x00, // Item Speed
	    0x00, 0x00, 0x00, 0x00, // ????
	    0xff, 0xff, 0xff, 0xff, // Item Selections 1
	    0xff, 0xff, 0xff, 0xff, // Item Selection 2
	    0x01, 0x01, 0x01, 0x01, // Rumble
	    0x00, 0x01, 0x01, 0x00, // Screen
	    0xF8, 0xFF, 0xFF, 0x4F, // Stages
	};

	u32 defaultStagesBlock = 0xE70000B0;

	std::vector<u8> RJJMatchBlock = defaultMatchBlock;

	std::unordered_map<u8, u32> payloadSizes = {
	    // The actual size of this command will be sent in one byte
	    // after the command is received. The other receive command IDs
	    // and sizes will be received immediately following
	    {CMD_RECEIVE_COMMANDS, 1},

	    // The following are all commands used to play back a replay and
	    // have fixed sizes
	    {CMD_PREPARE_REPLAY, 0},
	    {CMD_READ_FRAME, 4},
	    {CMD_IS_STOCK_STEAL, 5},
	    {CMD_GET_LOCATION, 6},
	    {CMD_IS_FILE_READY, 0},
	    {CMD_GET_GECKO_CODES, 0},

	    // The following are used for Slippi online and also have fixed sizes
	    {CMD_ONLINE_INPUTS, 17},
	    {CMD_CAPTURE_SAVESTATE, 32},
	    {CMD_LOAD_SAVESTATE, 32},
	    {CMD_GET_MATCH_STATE, 0},
	    {CMD_FIND_OPPONENT, 19},
	    {CMD_SET_MATCH_SELECTIONS, 6},
	    {CMD_SEND_CHAT_MESSAGE, 2},
	    {CMD_OPEN_LOGIN, 0},
	    {CMD_LOGOUT, 0},
	    {CMD_UPDATE, 0},
	    {CMD_GET_ONLINE_STATUS, 0},
	    {CMD_CLEANUP_CONNECTION, 0},
	    {CMD_GET_NEW_SEED, 0},
	    {CMD_REPORT_GAME, 16},
	    {CMD_SET_MATCH_INFO, 320},

	    // Misc
	    {CMD_LOG_MESSAGE, 0xFFFF}, // Variable size... will only work if by itself
	    {CMD_FILE_LENGTH, 0x40},
	    {CMD_FILE_LOAD, 0x40},
	    {CMD_GCT_LENGTH, 0x0},
	    {CMD_GCT_LOAD, 0x4},
	};

	struct WriteMessage
	{
		std::vector<u8> data;
		std::string operation;
	};

	// .slp File creation stuff
	u32 writtenByteCount = 0;

	// cout stuff
	bool outputCurrentFrame = false;
	bool shouldOutput = false;

	// vars for metadata generation
	time_t gameStartTime;
	s32 lastFrame;
	std::unordered_map<u8, std::unordered_map<u8, u32>> characterUsage;

	void updateMetadataFields(u8 *payload, u32 length);
	void configureCommands(u8 *payload, u8 length);
	void writeToFileAsync(u8 *payload, u32 length, std::string fileOption);
	void writeToFile(std::unique_ptr<WriteMessage> msg);
	std::vector<u8> generateMetadata();
	void createNewFile();
	void closeFile();
	std::string generateFileName();
	bool checkFrameFullyFetched(s32 frameIndex);
	bool shouldFFWFrame(s32 frameIndex);

	// std::ofstream log;

	File::IOFile m_file;
	std::vector<u8> m_payload;

	// online play stuff
	u16 getRandomStage();
	bool isDisconnected();
	void handleOnlineInputs(u8 *payload);
	void prepareOpponentInputs(u8 *payload);
	void handleSendInputs(u8 *payload);
	void handleCaptureSavestate(u8 *payload);
	void handleLoadSavestate(u8 *payload);
	void startFindMatch(u8 *payload);
	void prepareOnlineMatchState();
	void setMatchSelections(u8 *payload);
	bool shouldSkipOnlineFrame(s32 frame);
	void handleLogInRequest();
	void handleLogOutRequest();
	void handleUpdateAppRequest();
	void prepareOnlineStatus();
	void handleConnectionCleanup();
	void prepareNewSeed();
	void handleReportGame(u8 *payload);

	// replay playback stuff
	void prepareGameInfo(u8 *payload);
	void prepareGeckoList();
	void prepareCharacterFrameData(Slippi::FrameData *frame, u8 port, u8 isFollower);
	void prepareFrameData(u8 *payload);
	void prepareIsStockSteal(u8 *payload);
	void prepareIsFileReady();

	// misc stuff
	void setMatchInfo(u8 *payload);
	void handleChatMessage(u8 *payload);
	void logMessageFromGame(u8 *payload);
	void prepareFileLength(u8 *payload);
	void prepareFileLoad(u8 *payload);
	void prepareGctLength();
	void prepareGctLoad(u8 *payload);

	int getCharColor(u8 charId, u8 teamId);

	void FileWriteThread(void);

	Common::FifoQueue<std::unique_ptr<WriteMessage>, false> fileWriteQueue;
	bool writeThreadRunning = false;
	std::thread m_fileWriteThread;

	std::unordered_map<u8, std::string> getNetplayNames();

	std::vector<u8> playbackSavestatePayload;
	std::vector<u8> geckoList;

	u32 stallFrameCount = 0;
	bool isConnectionStalled = false;

	std::vector<u8> m_read_queue;
	std::unique_ptr<Slippi::SlippiGame> m_current_game = nullptr;
	SlippiSpectateServer *m_slippiserver = nullptr;
	SlippiMatchmaking::MatchSearchSettings lastSearch;

	std::vector<u16> stagePool;

	u32 frameSeqIdx = 0;

	bool isEnetInitialized = false;
	bool firstMatch = true;

	std::default_random_engine generator;

	// Frame skipping variables
	int framesToSkip = 0;
	bool isCurrentlySkipping = false;

	std::string forcedError = "";

	// Used to determine when to detect when a new session has started
	bool isPlaySessionActive = false;

  protected:
	void TransferByte(u8 &byte) override;

  private:
	SlippiPlayerSelections localSelections;

	std::unique_ptr<SlippiUser> user;
	std::unique_ptr<SlippiGameFileLoader> gameFileLoader;
	std::unique_ptr<SlippiNetplayClient> slippi_netplay;
	std::unique_ptr<SlippiMatchmaking> matchmaking;
	std::unique_ptr<SlippiGameReporter> gameReporter;

	std::map<s32, std::unique_ptr<SlippiSavestate>> activeSavestates;
	std::deque<std::unique_ptr<SlippiSavestate>> availableSavestates;
};
