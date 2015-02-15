#pragma once

struct NetClient
{
	struct _ENetPeer* m_enetpeer;
	struct _ENetHost * m_enetclient;

	void Initialize();
	void Connect(const char* connect);
	void Service();
};
