#pragma once

struct NetHost
{
	struct _ENetHost* m_enethost;

	void Initialize();
	void Create(size_t max_players);
	void Service();
};
