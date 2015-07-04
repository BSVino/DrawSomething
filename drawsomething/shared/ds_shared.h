#pragma once

#include "shared/tinker_shared.h"

#ifdef CLIENT_LIBRARY
#include "client/ds_client.h"
#include "client/net_client.h"
#else
#include "server/ds_server.h"
#include "server/net_host.h"
#endif

