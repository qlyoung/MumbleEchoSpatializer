/*
 * Mumble spatialization plugin for Echo VR.
 *
 * Copyright 2019 Quentin Young
 * All rights released.
 */
#include "httplib.h"
#include "json.hpp"
#include "mumble_plugin_main.h"
#include <fstream>
#include <string>
#include <iostream>

static int fetch(float *avatar_pos, float *avatar_front, float *avatar_top, float *camera_pos, float *camera_front, float *camera_top, std::string &context, std::wstring &identity)
{
	/* open log file */
	using json = nlohmann::json;

	for (int i = 0; i < 3; i++) {
		avatar_pos[i] = avatar_front[i] = avatar_top[i] = camera_pos[i] = camera_front[i] = camera_top[i] = 0.0f;
	}

	/* Fetch API data */
	httplib::Client echoclient("127.0.0.1", 80);
	auto res = echoclient.Get("/session");
	if (!res || res->status != 200) {
		return false;
	}

	/* Parse as JSON */
	std::cout << res->body << std::endl;
	auto json_body = json::parse(res->body);

	/* If not playing, return zeroes */
	if (json_body["game_status"].get<std::string>() != "playing") {
		context.clear();
		identity.clear();
		for (int i = 0; i < 3; i++)
			avatar_pos[i] = avatar_front[i] = avatar_top[i] = camera_pos[i] = camera_front[i] = camera_top[i] = 0.0f;

		return true;
	}

	/* Get player position vectors */
	auto teams = json_body["teams"];
	auto pname = json_body["client_name"].get<std::string>();
	json pos, forward, up;
	bool found = false;
	for (json::iterator it = teams.begin(); it != teams.end() && !found; ++it) {
		auto players = (*it)["players"];
		for (json::iterator p = players.begin(); p != players.end(); ++p) {
			if ((*p)["name"].get<std::string>() == pname) {
				pos = (*p)["position"];
				forward = (*p)["forward"];
				up = (*p)["up"];
				found = true;
			}
		}
	}

	if (!found)
		return false;

	/* Map game vectors to Mumble vectors */
	avatar_pos[0] = -pos[0].get<float>();
	avatar_pos[1] = pos[1].get<float>();
	avatar_pos[2] = pos[2].get<float>();

	avatar_front[0] = -forward[0].get<float>();
	avatar_front[1] = forward[1].get<float>();
	avatar_front[2] = forward[2].get<float>();

	avatar_top[0] = -up[0].get<float>();
	avatar_top[1] = up[1].get<float>();
	avatar_top[2] = up[2].get<float>();

	// Sync camera with avatar
	for (int i = 0; i < 3; i++) {
		camera_pos[i] = avatar_pos[i];
		camera_front[i] = avatar_front[i];
		camera_top[i] = avatar_top[i];
	}

	return true;
}

static int trylock(const std::multimap<std::wstring, unsigned long long int> &pids)
{
	return true;
}

void unlock() {};

#define ECHO_VERSION L"(latest)"

static const std::wstring longdesc()
{
	return std::wstring(L"Supports Echo " ECHO_VERSION);
}

static std::wstring description(L"Echo VR " ECHO_VERSION);
static std::wstring shortname(L"Echo VR");

static int trylock1()
{
	return trylock(std::multimap<std::wstring, unsigned long long int>());
}

static MumblePlugin gameplug = {
	MUMBLE_PLUGIN_MAGIC,
	description,
	shortname,
	NULL,
	NULL,
	trylock1,
	unlock,
	longdesc,
	fetch
};

static MumblePlugin2 gameplug2 = {
	MUMBLE_PLUGIN_MAGIC_2,
	MUMBLE_PLUGIN_VERSION,
	trylock
};

extern "C" MUMBLE_PLUGIN_EXPORT MumblePlugin *getMumblePlugin()
{
	return &gameplug;
}

extern "C" MUMBLE_PLUGIN_EXPORT MumblePlugin2 *getMumblePlugin2()
{
	return &gameplug2;
}