/**********************************************************************
 *
 * Solution   : Euro Truck Multiplayer
 * Project    : Euro Truck Multiplayer - DLL
 * Licence    : GNU GPL v3
 * File       : CNetworkManager.cpp
 * Description: None
 * Developers : 
 *              FRi <FRi.developing@gmail.com>
 *
 **********************************************************************/
#include "StdInc.h"

extern CClient * g_pClient;

CNetworkManager::CNetworkManager()
{
	// Initialize RakPeer.
	m_pRakPeer = RakNet::RakPeerInterface::GetInstance();

	// Initialize RPC4 RakNet plugin.
	m_pRPC4 = RakNet::RPC4::GetInstance();

	// Set connection to status to 0;
	m_bConnected = false;

	// Start interfejsu menad�era graczy
	m_pPlayerManager = new CPlayerManager();

	// Start menad�era serwera.
	m_pServerManager = new CServerManager();
}

CNetworkManager::~CNetworkManager()
{
	if(IsConnected())
		Disconnect();
}

void CNetworkManager::Connect(const char*szHost, unsigned int uiPort, const char*szNickName)
{
	// Safe disconnect.
	if ( IsConnected() )
		Disconnect();

	//przypisanie danych
	m_uiPort = uiPort;
	m_strHost.Set(szHost);

	// Startup RakNet.
	m_pRakPeer->Startup(1, &RakNet::SocketDescriptor(), 1); 

	// Clear server address.
	m_ServerAddress = RakNet::UNASSIGNED_SYSTEM_ADDRESS;

	// Attach RPC4 to RakPeer.
	m_pRakPeer->AttachPlugin(m_pRPC4);
	//m_pRPCManager = new CRPCManager(m_pRakPeer);

	// Initialize PacketManager handler.
	m_pPacketManager = new CPacketManager(m_pRPC4/*m_pRPCManager*/);

	// Install RPC4 Packets.
	m_pPacketManager->Install();
	
	// Message "Connection to IP:PORT..."
	CString szMessage("��czenie z ip %s:%d..", szHost, uiPort, szNickName);
	g_pClient->GetChatbox()->AddChatMessage(szMessage.Get(), 0xFF0000FF);

	// Ustawianie nicku.
	m_pPlayerManager->GetLocalPlayer()->SetName(CString(szNickName));

	// Connecting
	m_pRakPeer->Connect(m_strHost.Get(), m_uiPort, NULL, NULL); 
}

bool CNetworkManager::Call( const char* uniqueID, RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool broadcast )
{
	if(IsConnected())
	{
		m_pRPC4->Call(uniqueID, bitStream, priority, reliability, orderingChannel, RakNet::UNASSIGNED_SYSTEM_ADDRESS, broadcast);
		return true;
	}
	return false;
}

bool CNetworkManager::Disconnect ( )
{
	// Check connection state.
	if ( IsConnected ( ) )
	{	
		// Disconnect from host.
		m_pRakPeer->CloseConnection(m_ServerAddress, true);

		//Usuwamy menad�er pakiet�w.
		m_pPacketManager->Uninstall();

		// Wywalamy zarz�dzanie pakietemi.
		m_pRakPeer->DetachPlugin(m_pRPC4);

		// Shoutdown RakPeer
		m_pRakPeer->Shutdown(0);

		//Usuwamy interfejsy
		RakNet::RPC4::DestroyInstance(m_pRPC4);
		RakNet::RakPeerInterface::DestroyInstance(m_pRakPeer);

		// Ustalanie stanu po��czenia.
		SetConnected(false);
		return true;
	}
	return false;
}

void CNetworkManager::Process()
{
	// Tworzenie uchwytu dla odbieranego pakietu.
	RakNet::Packet * pPacket = NULL;

	// P�tla dla menad�era graczy;
	if(m_pPlayerManager)
		m_pPlayerManager->Process();

	// While for all receives packets.
	while(pPacket = m_pRakPeer->Receive())
	{
		// Get packet uid.
		switch(pPacket->data[0])
		{
		case ID_CONNECTION_REQUEST_ACCEPTED:
			// Wywo�ywanie funkcji o po��czeniu.
			ConnectionSuccess(pPacket);
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			g_pClient->GetChatbox()->AddDebugMessage("Serwer zosta� wy�aczony. Trwa ponowne ��czenie..");
			m_pRakPeer->Connect(m_strHost.Get(), m_uiPort, NULL, NULL); 
			break;
		case ID_CONNECTION_LOST:
			g_pClient->GetChatbox()->AddDebugMessage("Zgubiono po�aczeni z serwerem. Trwa ponowne ��czenie..");
			m_pRakPeer->Connect(m_strHost.Get(), m_uiPort, NULL, NULL); 
			break;
		case ID_CONNECTION_ATTEMPT_FAILED:
			g_pClient->GetChatbox()->AddDebugMessage("Po��cznie nie powiod�o si�. Trwa ponowne ��czenie..");
			m_pRakPeer->Connect(m_strHost.Get(), m_uiPort, NULL, NULL); 
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			g_pClient->GetChatbox()->AddDebugMessage("Serwer jest pe�ny. Trwa ponowne ��czenie..");
			m_pRakPeer->Connect(m_strHost.Get(), m_uiPort, NULL, NULL); 
			break;
		}

		// Deloccate packet.
		m_pRakPeer->DeallocatePacket(pPacket);	
	}
}

void CNetworkManager::ConnectionSuccess(RakNet::Packet *pPacket)
{
	// Dodanie wiadomo�ci o po��czeniu do czatu.
	g_pClient->GetChatbox()->AddDebugMessage("Po��czono pomy�lnie!");

	// Ustalenie aktualnego adresu serwera
	SetServerAddress(pPacket->systemAddress);
	
	//Ustawianie informacji o stanie po��czenia.
	SetConnected(true);

	// Wys�anie pakietu z podstawowymi informacjami o graczu.
	RakNet::BitStream bitStream;
	bitStream.Write(m_pPlayerManager->GetLocalPlayer()->GetName().Get(), MAX_PLAYER_NAME);
	Call("Player_Connect", &bitStream, HIGH_PRIORITY, RELIABLE, 0, true);
}