// Copyright 2010 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include "Common/CommonTypes.h"
#include "Common/Event.h"
#include "Common/FifoQueue.h"
#include "Common/Timer.h"
#include "Common/TraversalClient.h"
#include "Core/NetPlayProto.h"
#include "Core/Slippi/SlippiPad.h"
#include "InputCommon/GCPadStatus.h"
#include <SFML/Network/Packet.hpp>
#include <array>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#ifdef _WIN32
#include <Qos2.h>
#endif

#define SLIPPI_ONLINE_LOCKSTEP_INTERVAL 30 // Number of frames to wait before attempting to time-sync
#define SLIPPI_PING_DISPLAY_INTERVAL 60

struct SlippiRemotePadOutput
{
	int32_t latestFrame;
	std::vector<u8> data;
};

class SlippiPlayerSelections
{
  public:
	u8 characterId = 0;
	u8 characterColor = 0;
	bool isCharacterSelected = false;

	u16 stageId = 0;
	bool isStageSelected = false;
	std::vector<u8> matchRules;

	u32 rngOffset = 0;

	int messageId;

	void Merge(SlippiPlayerSelections &s)
	{
		this->rngOffset = s.rngOffset;

		if (s.isStageSelected)
		{
			this->stageId = s.stageId;
			this->isStageSelected = true;
		}

		if (s.isCharacterSelected)
		{
			this->characterId = s.characterId;
			this->characterColor = s.characterColor;
			this->isCharacterSelected = true;

			this->matchRules = s.matchRules;
		}


	}

	void Reset()
	{
		characterId = 0;
		characterColor = 0;
		isCharacterSelected = false;

		stageId = 0;
		isStageSelected = false;

		rngOffset = 0;
	}
};

class SlippiMatchInfo
{
  public:
	SlippiPlayerSelections localPlayerSelections;
	SlippiPlayerSelections remotePlayerSelections;

	void Reset()
	{
		localPlayerSelections.Reset();
		remotePlayerSelections.Reset();
	}
};

class SlippiNetplayClient
{
  public:
	void ThreadFunc();
	void SendAsync(std::unique_ptr<sf::Packet> packet);

	SlippiNetplayClient(bool isDecider); // Make a dummy client
	SlippiNetplayClient(const std::string &address, const u16 remotePort, const u16 localPort, bool isDecider);
	~SlippiNetplayClient();

	// Slippi Online
	enum class SlippiConnectStatus
	{
		NET_CONNECT_STATUS_UNSET,
		NET_CONNECT_STATUS_INITIATED,
		NET_CONNECT_STATUS_CONNECTED,
		NET_CONNECT_STATUS_FAILED,
		NET_CONNECT_STATUS_DISCONNECTED,
	};

	bool IsDecider();
	bool IsConnectionSelected();
	SlippiConnectStatus GetSlippiConnectStatus();
	void StartSlippiGame();
	void SendConnectionSelected();
	void SendSlippiPad(std::unique_ptr<SlippiPad> pad);
	void SetMatchSelections(SlippiPlayerSelections &s);
	std::unique_ptr<SlippiRemotePadOutput> GetSlippiRemotePad(int32_t curFrame);
	SlippiMatchInfo *GetMatchInfo();
	u64 GetSlippiPing();
	int32_t GetSlippiLatestRemoteFrame();
	u8 GetSlippiRemoteChatMessage();
	u8 GetSlippiRemoteSentChatMessage();
	s32 CalcTimeOffsetUs();

	void WriteChatMessageToPacket(sf::Packet &packet, int messageId);
	std::unique_ptr<SlippiPlayerSelections> ReadChatMessageFromPacket(sf::Packet &packet);

	u8 remoteChatMessageId = 0;     // most recent chat message id from opponent
	u8 remoteSentChatMessageId = 0; // most recent chat message id that current player sent

  protected:
	struct
	{
		std::recursive_mutex game;
		// lock order
		std::recursive_mutex players;
		std::recursive_mutex async_queue_write;
	} m_crit;

	Common::FifoQueue<std::unique_ptr<sf::Packet>, false> m_async_queue;

	ENetHost *m_client = nullptr;
	ENetPeer *m_server = nullptr;
	std::thread m_thread;

	std::string m_selected_game;
	Common::Flag m_is_running{false};
	Common::Flag m_do_loop{true};

	unsigned int m_minimum_buffer_size = 6;

	u32 m_current_game = 0;

	// Slippi Stuff
	struct FrameTiming
	{
		int32_t frame;
		u64 timeUs;
	};

	struct
	{
		// TODO: Should the buffer size be dynamic based on time sync interval or not?
		int idx;
		std::vector<s32> buf;
	} frameOffsetData;

	bool isConnectionSelected = false;
	bool isDecider = false;
	int32_t lastFrameAcked;
	bool hasGameStarted = false;
	FrameTiming lastFrameTiming;
	u64 pingUs;

	std::deque<std::unique_ptr<SlippiPad>> localPadQueue;  // most recent inputs at start of deque
	std::deque<std::unique_ptr<SlippiPad>> remotePadQueue; // most recent inputs at start of deque
	Common::FifoQueue<FrameTiming, false> ackTimers;
	SlippiConnectStatus slippiConnectStatus = SlippiConnectStatus::NET_CONNECT_STATUS_UNSET;
	SlippiMatchInfo matchInfo;

	bool m_is_recording = false;

	void writeToPacket(sf::Packet &packet, SlippiPlayerSelections &s);
	std::unique_ptr<SlippiPlayerSelections> readSelectionsFromPacket(sf::Packet &packet);

  private:
	unsigned int OnData(sf::Packet &packet);
	void Send(sf::Packet &packet);
	void Disconnect();

	bool m_is_connected = false;

#ifdef _WIN32
	HANDLE m_qos_handle;
	QOS_FLOWID m_qos_flow_id;
#endif

	u32 m_timebase_frame = 0;
};
extern SlippiNetplayClient* SLIPPI_NETPLAY; // singleton static pointer

static bool IsOnline(){
    return SLIPPI_NETPLAY != nullptr;
}