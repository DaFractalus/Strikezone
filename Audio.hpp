#pragma once 
#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <AL/al.h>
#include <AL/alc.h>
#include "Toolkit.hpp"

namespace Engine { class GameEngine; }

namespace TLB
{
	bool check_al_error(const char* what);

	class Sound
	{
	public:
		int channels, samples, sampleRate;
		bool loaded;
		ALuint buffer;

		Sound();
		void Load(const char* path, bool forceMono);
		operator ALuint() const { return buffer; }
		~Sound();
	};

	class SoundSrcPool
	{
		uint sourceCount;
		std::vector<ALuint> sourcePool;
		std::vector<bool> sourceInUse;

	public:

		SoundSrcPool(uint maxSourceCount);
		void GenerateSources();
		ALuint GetFreeSource();
		void ReclaimSources();
	};

	class AudioManager;
	class TrackInstance
	{
		float pitch = 1.f, volume = 1.f;
		int trackid = -1, _handle = -1;

	public:
		AudioManager* _manager = nullptr;

		TrackInstance(AudioManager& audioManager);

		TrackInstance(const TrackInstance&) = delete;
		TrackInstance& operator=(const TrackInstance&) = delete;

		TrackInstance(TrackInstance&& other) noexcept;
		TrackInstance& operator=(TrackInstance&& other) noexcept;

		~TrackInstance();

		void SetVolume(float Volume);
		void SetPitch(float Pitch);
		void SetTrack(int id);

		float GetVolume() const;
		float GetPitch() const;
		int GetTrackID() const;
		int GetHandle() const;

	};

	struct ActiveTrack
	{
		float volume = 0.f;
		float pitch = -1.f;
		const int trackid;

		ActiveTrack(TrackInstance& instance);
		void Update(AudioManager& manager, float dt);
		~ActiveTrack();

		TrackInstance* instance = nullptr;
		float targetVolume = 1.f;
		float targetPitch = 1.f;
		bool toBeDeactivated = false;

	private:
		ALuint _source = 0;
	};


	class AudioManager
	{
		ALCdevice* audioDevice;
		ALCcontext* audioContext;
		std::unordered_map<int, Sound> _sounds;
		SoundSrcPool srcPool;
		RNG rng;
		int latestHandle;
		std::unordered_map<int, TrackInstance*> trackInstances; // int -> handle
		std::unordered_map<int, ActiveTrack> activeTracks; // int -> trackid

	public:

		AudioManager();
		void LoadSound(int id, const char* path, bool forceMono = false);
		void PlaySound(int id, float vol, float pitch, float pitchOffset, glm::vec3 pos, glm::vec3 vel, bool relative = false, float rolloffFactor = 1.f, float maxDist = 50.f, float referenceDist = 3.f);
		void PlayUISound(int id, float vol = 1.f, float pitch = 1.f, float pitchOffset = 0.f);
		ALuint GetSoundBuffer(int trackid);
		float GetSoundLength(int id);
		void UpdateListener(glm::vec3 pos, glm::vec3 vel, glm::vec3 up, glm::vec3 forward);
		void UpdateSourcesAndTracks(Engine::GameEngine& game);

		int GetNextHandle();
		void PlaceTrackInstance(int handle, TrackInstance* instance);
		void DeactivateTrack(int trackid);

		~AudioManager();

	};

}

#endif
