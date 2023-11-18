#pragma once

#ifndef _ENV_CHECK_
#define _ENV_CHECK_

#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

#if __GNUC__
#if __x84_64__ || __ppc_64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif
#endif