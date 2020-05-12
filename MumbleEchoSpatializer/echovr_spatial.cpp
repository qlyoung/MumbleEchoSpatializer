/*
 * Mumble spatialization plugin for Echo VR.
 * Copyright 2019 Quentin Young
 *
 * ---
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org/>
 */
#include "httplib.h"
#include "json.hpp"
#include "mumble_plugin_main.h"

#define ECHOVR_WINDOWNAME "Echo VR"
#define ECHO_VERSION L"27.0.439148.0+"
#define THIS_PLUGIN_VERSION L"v2.0.0"
#define UPDATE_FREQUENCY 60

std::thread fetchthread;
std::mutex mtx;
std::atomic<bool> running;

struct EchoPositionState {
	float avatar_front[3];
	float avatar_pos[3];
	float avatar_top[3];
	float camera_pos[3];
	float camera_front[3];
	float camera_top[3];
	std::wstring identity;
	std::string context;
	bool noresp;
	bool dead;
} latestState;

bool is_echo_running()
{
	return (bool)FindWindowA(NULL, ECHOVR_WINDOWNAME);
}

static inline void update(EchoPositionState &p)
{
	mtx.lock();
	{
		latestState = p;
	}
	mtx.unlock();
}

static void fetchloop()
{
	httplib::Client echoclient("127.0.0.1", 6721, 1);
	using json = nlohmann::json;

	auto start = std::chrono::steady_clock::now();
	auto base = std::chrono::milliseconds(1000 / UPDATE_FREQUENCY);

	while (running) {
		auto sleeptime = std::max(
			std::chrono::milliseconds(0),
			std::chrono::duration_cast<std::chrono::milliseconds>(
				base
				- (std::chrono::steady_clock::now() - start)));
		std::this_thread::sleep_for(sleeptime);
		start = std::chrono::steady_clock::now();

		auto res = echoclient.Get("/session");

		EchoPositionState p = {};

		if (!res || res->status != 200) {
			p.noresp = true;
			p.dead = !is_echo_running();
			update(p);
			continue;
		}


		auto json_body = json::parse(res->body);

		auto state = json_body["game_status"].get<std::string>();
		auto client_name = json_body["client_name"].get<std::string>();
		auto sessionid = json_body["sessionid"].get<std::string>();

		p.identity =
			std::wstring(client_name.begin(), client_name.end());
		p.context = sessionid;

		json pos, forward, up;

		auto teams = json_body["teams"];
		bool found = false;
		for (json::iterator t = teams.begin();
		     t != teams.end() && !found; ++t) {
			auto players = (*t)["players"];
			for (json::iterator p = players.begin();
			     p != players.end(); ++p) {
				if ((*p)["name"].get<std::string>()
				    == client_name) {
					pos = (*p)["position"];
					forward = (*p)["forward"];
					up = (*p)["up"];
					found = true;
				}
			}
		}

		if (!found) {
			update(p);
			continue;
		}


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

		update(p);
	}
}

static int fetch(float *avatar_pos, float *avatar_front, float *avatar_top,
		 float *camera_pos, float *camera_front, float *camera_top,
		 std::string &context, std::wstring &identity)
{

	for (int i = 0; i < 3; i++) {
		avatar_pos[i] = avatar_front[i] = avatar_top[i] =
			camera_pos[i] = camera_front[i] = camera_top[i] = 0.0f;
	}

	mtx.lock();
	struct EchoPositionState res = latestState;
	mtx.unlock();

	if (res.dead) {
		return false;
	}

	if (res.noresp) {
		context.clear();
		return true;
	}

	context = res.context;
	identity = res.identity;
	memcpy(avatar_pos, &res.avatar_pos, sizeof(res.avatar_pos));
	memcpy(avatar_front, &res.avatar_front, sizeof(res.avatar_front));
	memcpy(avatar_top, &res.avatar_top, sizeof(res.avatar_top));
	memcpy(camera_pos, &res.camera_pos, sizeof(res.camera_pos));
	memcpy(camera_front, &res.camera_front, sizeof(res.camera_front));
	memcpy(camera_top, &res.camera_top, sizeof(res.camera_top));

	return true;
}

static int
trylock(const std::multimap<std::wstring, unsigned long long int> &pids)
{
	if (is_echo_running() && !running) {
		running = true;
		fetchthread = std::thread(fetchloop);
		return true;
	}

	return false;
}

void unlock()
{
	running = false;
	fetchthread.join();
};

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

static MumblePlugin gameplug = {MUMBLE_PLUGIN_MAGIC,
				description,
				shortname,
				NULL,
				NULL,
				trylock1,
				unlock,
				longdesc,
				fetch};

static MumblePlugin2 gameplug2 = {MUMBLE_PLUGIN_MAGIC_2, MUMBLE_PLUGIN_VERSION,
				  trylock};

extern "C" MUMBLE_PLUGIN_EXPORT MumblePlugin *getMumblePlugin()
{
	return &gameplug;
}

extern "C" MUMBLE_PLUGIN_EXPORT MumblePlugin2 *getMumblePlugin2()
{
	return &gameplug2;
}
