/*
 * Mumble spatialization plugin for Echo VR.
 *
 * Copyright 2019 Quentin Young
 * All rights released.
 */
#include "httplib.h"
#include "json.hpp"
#include "mumble_plugin_main.h"

std::thread fetchthread;
std::mutex mtx;
std::atomic<bool> running;

struct LastPositionals {
	float avatar_front[3];
	float avatar_pos[3];
	float avatar_top[3];
	float camera_pos[3];
	float camera_front[3];
	float camera_top[3];
	bool clear;
	bool noresp;
} lastResult;

static void fetchloop()
{
	httplib::Client echoclient("127.0.0.1", 80);
	using json = nlohmann::json;

	while (running) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
		auto res = echoclient.Get("/session");
		std::vector<std::string> pos_states{ "playing", "score", "round_start", "round_over" };

		if (!res || res->status != 200) {
			mtx.lock();
			{
				lastResult = {};
				lastResult.noresp = true;
			}
			mtx.unlock();
			continue;
		}

		auto json_body = json::parse(res->body);
		std::string state = json_body["game_status"].get<std::string>();

		if (std::find(std::begin(pos_states), std::end(pos_states), state) == std::end(pos_states)) {
			mtx.lock();
			{
				lastResult = {};
				lastResult.clear = true;
			}
			mtx.unlock();
			continue;
		}

		auto teams = json_body["teams"];
		auto pname = json_body["client_name"].get<std::string>();
		json pos, forward, up;
		bool found = false;
		for (json::iterator t = teams.begin(); t != teams.end() && !found; ++t) {
			auto players = (*t)["players"];
			for (json::iterator p = players.begin(); p != players.end(); ++p) {
				if ((*p)["name"].get<std::string>() == pname) {
					pos = (*p)["position"];
					forward = (*p)["forward"];
					up = (*p)["up"];
					found = true;
				}
			}
		}

		if (!found) {
			mtx.lock();
			{
				lastResult = {};
				lastResult.clear = true;
			}
			mtx.unlock();
			continue;
		}

		LastPositionals p = {};

		p.avatar_pos[0] = -pos[0].get<float>();
		p.avatar_pos[1] = pos[1].get<float>();
		p.avatar_pos[2] = pos[2].get<float>();

		p.avatar_front[0] = -forward[0].get<float>();
		p.avatar_front[1] = forward[1].get<float>();
		p.avatar_front[2] = forward[2].get<float>();

		p.avatar_top[0] = -up[0].get<float>();
		p.avatar_top[1] = up[1].get<float>();
		p.avatar_top[2] = up[2].get<float>();

		for (int i = 0; i < 3; i++) {
			p.camera_pos[i] = p.avatar_pos[i];
			p.camera_front[i] = p.avatar_front[i];
			p.camera_top[i] = p.avatar_top[i];
		}

		mtx.lock();
		{
			lastResult = p;
		}
		mtx.unlock();
	}
}

static int fetch(float *avatar_pos, float *avatar_front, float *avatar_top, float *camera_pos, float *camera_front, float *camera_top, std::string &context, std::wstring &identity)
{

	for (int i = 0; i < 3; i++) {
		avatar_pos[i] = avatar_front[i] = avatar_top[i] = camera_pos[i] = camera_front[i] = camera_top[i] = 0.0f;
	}

	mtx.lock();
	struct LastPositionals res = lastResult;
	mtx.unlock();

	if (res.noresp)
		return false;

	if (res.clear) {
		context.clear();
		identity.clear();
		return true;
	}

	memcpy(avatar_pos, &res.avatar_pos, sizeof(res.avatar_pos));
	memcpy(avatar_front, &res.avatar_front, sizeof(res.avatar_front));
	memcpy(avatar_top, &res.avatar_top, sizeof(res.avatar_top));
	memcpy(camera_pos, &res.camera_pos, sizeof(res.camera_pos));
	memcpy(camera_front, &res.camera_front, sizeof(res.camera_front));
	memcpy(camera_top, &res.camera_top, sizeof(res.camera_top));

	return true;
}

static int trylock(const std::multimap<std::wstring, unsigned long long int> &pids)
{
	if (!running) {
		running = true;
		fetchthread = std::thread(fetchloop);
	}
	return true;
}

void unlock() {
	running = false;
	fetchthread.join();
};

#define ECHO_VERSION L"(latest)"
#define THIS_PLUGIN_VERSION L"v0.1.1"

static const std::wstring longdesc()
{
	return std::wstring(L"Supports Echo " ECHO_VERSION);
}

static std::wstring description(L"Echo VR " THIS_PLUGIN_VERSION);
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