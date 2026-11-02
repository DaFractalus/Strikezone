#include "Audio.hpp"
#include "GameEngine.hpp"

namespace TLB
{
    bool check_al_error(const char* what)
    {
        ALenum err = alGetError();
        if (err != AL_NO_ERROR) {
            std::cerr << "[OpenAL Error] " << what << ": " << alGetString(err) << "\n";
            return true;
        }
        return false;
    }

    // --- Sound ---
    Sound::Sound() : channels(0), sampleRate(0), loaded(false), buffer(0), samples(0)
    {
        alGenBuffers(1, &buffer);
    }

    void Sound::Load(const char* path, bool forceMono)
    {
        loaded = false;
        short* decoded;
        samples = stb_vorbis_decode_filename(path, &channels, &sampleRate, &decoded);
        if (samples < 0) { std::cerr << "[Sound] Failed to decode: " << path << "\n"; return; }

        ALenum format;
        if (channels == 1 || forceMono) { format = AL_FORMAT_MONO16; }
        else if (channels == 2) { format = AL_FORMAT_STEREO16; }
        else
        {
            std::cerr << "[Sound] Unsupported channel count: " << channels << "\n";
            free(decoded);
            return;
        }

        // Create OpenAL buffer
        if (check_al_error("alGenBuffers")) { free(decoded); return; }

        //Downmix if sound is stereo and mono is forced
        if (channels == 2 && forceMono)
        {
            for (int i = 0; i < samples; ++i)
            {
                int left = decoded[i * 2];
                int right = decoded[i * 2 + 1];
                decoded[i] = static_cast<short>((left + right) / 2);
            }
            channels = 1;
        }

        // Upload data
        alBufferData(buffer, format, decoded, samples * channels * sizeof(short), sampleRate);

        if (check_al_error("alBufferData"))
        {
            free(decoded);
            return;
        }

        // stb_vorbis allocates internally
        free(decoded);
        loaded = true;
        //std::cout << "[Sound] Loaded " << path << " (" << channels << " ch, " << sampleRate << "Hz, " << samples << " samples)\n";
    }

    Sound::~Sound()
    {
        if (buffer != 0) { alDeleteBuffers(1, &buffer); }
    }

    // --- SoundSrcPool ---
    SoundSrcPool::SoundSrcPool(uint maxSourceCount) : sourceCount(maxSourceCount)
    {
        sourcePool.resize(maxSourceCount);
        sourceInUse.resize(maxSourceCount, false);
    }

    void SoundSrcPool::GenerateSources()
    {
        alGenSources(sourceCount, sourcePool.data());
    }

    ALuint SoundSrcPool::GetFreeSource()
    {
        for (int i = 0; i < sourceCount; ++i)
        {
            if (!sourceInUse[i]) { sourceInUse[i] = true; return sourcePool[i]; }
        }
        return 0; // No free sources available
    }

    void SoundSrcPool::ReclaimSources()
    {
        for (int i = 0; i < sourcePool.size(); ++i)
        {
            if (sourceInUse[i])
            {
                ALint state;
                alGetSourcei(sourcePool[i], AL_SOURCE_STATE, &state);
                if (state == AL_STOPPED) {
                    alSourcei(sourcePool[i], AL_BUFFER, 0); // detach buffer
                    sourceInUse[i] = false;
                }
            }
        }
    }

    // --- TrackInstance ---
    TrackInstance::TrackInstance(AudioManager& audioManager) : _manager(&audioManager), _handle(audioManager.GetNextHandle())
    {
        audioManager.PlaceTrackInstance(_handle, this);
    }

    TrackInstance::TrackInstance(TrackInstance&& other) noexcept
    {
        _manager = other._manager;
        pitch = other.pitch;
        volume = other.volume;
        trackid = other.trackid;
        _handle = other._handle;

        other.trackid = -1;
        other._handle = -1;

        _manager->PlaceTrackInstance(_handle, this);
    }

    TrackInstance& TrackInstance::operator=(TrackInstance&& other) noexcept
    {
        _manager = other._manager;
        pitch = other.pitch;
        volume = other.volume;
        trackid = other.trackid;

        other.trackid = -1;
        other._handle = -1;

        _manager->PlaceTrackInstance(_handle, this);
        return *this;
    }

    TrackInstance::~TrackInstance()
    {
        _manager->PlaceTrackInstance(_handle, nullptr);
    }

    void TrackInstance::SetVolume(float Volume) { volume = glm::max(0.f, Volume); }
    void TrackInstance::SetPitch(float Pitch) { pitch = glm::max(0.f, Pitch); }
    void TrackInstance::SetTrack(int id)
    {
        trackid = _manager->GetSoundBuffer(id) == 0 ? -1 : id;
        _manager->PlaceTrackInstance(_handle, this);
    }

    float TrackInstance::GetVolume()  const { return volume; }
    float TrackInstance::GetPitch()  const { return pitch; }
    int TrackInstance::GetTrackID() const { return trackid; }
    int TrackInstance::GetHandle() const { return _handle; }

    // --- ActiveTrack ---
    ActiveTrack::ActiveTrack(TrackInstance& instance) : trackid(instance.GetTrackID()),
        targetPitch(instance.GetPitch()), targetVolume(instance.GetVolume())
    {
        pitch = targetPitch;
        alGenSources(1, &_source);
        alSourcei(_source, AL_BUFFER, instance._manager->GetSoundBuffer(trackid));
        alSourcei(_source, AL_LOOPING, AL_TRUE);
        alSourcef(_source, AL_GAIN, 0.f);
        alSourcef(_source, AL_PITCH, pitch);
        alSourcei(_source, AL_SOURCE_RELATIVE, AL_TRUE);
        alSource3f(_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
        alSource3f(_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
        alSourcef(_source, AL_ROLLOFF_FACTOR, 0.0f);
        alSourcePlay(_source);
    }

    void ActiveTrack::Update(AudioManager& manager, float dt)
    {
        float dV = dt * .75f;
        if (instance == nullptr)
        {
            volume -= dV;
            if (volume < 0.f) toBeDeactivated = true;
        }
        else
        {
            toBeDeactivated = false;
            targetVolume = instance->GetVolume();
            targetPitch = instance->GetPitch();

            if (volume < targetVolume)      volume += glm::min(dV, targetVolume - volume);
            else if (volume > targetVolume) volume -= glm::min(dV, volume - targetVolume);

            if (pitch < 0) pitch = targetPitch;
            else
            {
                if (pitch < targetPitch)      pitch += glm::min(dV, targetPitch - pitch);
                else if (pitch > targetPitch) pitch -= glm::min(dV, pitch - targetPitch);
            }
        }

        alSourcef(_source, AL_GAIN, glm::max(0.f, volume));
        alSourcef(_source, AL_PITCH, pitch);

    }

    ActiveTrack::~ActiveTrack()
    {
        if (!_source) alDeleteSources(1, &_source);
    }

    // --- AudioManager ---
    AudioManager::AudioManager() : audioDevice(nullptr), audioContext(nullptr),
        srcPool(32), latestHandle(0)
    {
        audioDevice = alcOpenDevice(nullptr); // default device
        if (!audioDevice) { std::cerr << "[AudioManager] Failed to open OpenAL device\n"; return; }

        audioContext = alcCreateContext(audioDevice, nullptr);
        if (!audioContext) { std::cerr << "[AudioManager] Failed to create OpenAL context\n"; alcCloseDevice(audioDevice); return; }

        alcMakeContextCurrent(audioContext);
        std::cout << "[AudioManager] OpenAL initialized: " << alcGetString(audioDevice, ALC_ALL_DEVICES_SPECIFIER) << "\n";

        srcPool.GenerateSources();
    }

    void AudioManager::LoadSound(int id, const char* path, bool forceMono)
    {
        _sounds[id].Load(path, forceMono);
        if (!_sounds.at(id).loaded) _sounds.erase(id);
    }

    void AudioManager::PlaySound(int id, float vol, float pitch, float pitchOffset, glm::vec3 pos, glm::vec3 vel, bool relative, float rolloffFactor, float maxDist, float referenceDist)
    {
        if (!_sounds.contains(id)) return;
        ALuint src = srcPool.GetFreeSource();
        if (src != 0)
        {
            alSourcei(src, AL_BUFFER, _sounds.at(id));
            alSourcef(src, AL_GAIN, vol);
            alSourcef(src, AL_PITCH, pitch + (pitchOffset == 0 ? 0 : (1 - 2 * rng.NextFloat()) * pitchOffset));
            alSource3f(src, AL_POSITION, pos.x, pos.y, pos.z);
            alSource3f(src, AL_VELOCITY, vel.x, vel.y, vel.z);

            alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
            alSourcei(src, AL_SOURCE_RELATIVE, relative);
            alSourcef(src, AL_MAX_DISTANCE, maxDist);
            alSourcef(src, AL_REFERENCE_DISTANCE, referenceDist);
            alSourcef(src, AL_ROLLOFF_FACTOR, rolloffFactor);

            alSourcePlay(src);
        }
    }

    void AudioManager::PlayUISound(int id, float vol, float pitch, float pitchOffset)
    {
        PlaySound(id, vol, pitch, pitchOffset, { 0, 0, 0 }, { 0, 0, 0 }, true, 0, 1, 1);
    }

    ALuint AudioManager::GetSoundBuffer(int trackid)
    {
        auto it = _sounds.find(trackid);
        if (it != _sounds.end()) return it->second;
        return 0u;
    }

    float AudioManager::GetSoundLength(int id)
    {
        if (!_sounds.contains(id)) return -1;
        auto& sound = _sounds.at(id);
        return sound.samples / (float)sound.sampleRate;
    }

    void AudioManager::UpdateListener(glm::vec3 pos, glm::vec3 vel, glm::vec3 up, glm::vec3 forward)
    {
        // Update each frame from camera
        alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
        alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);
        float orientation[6] = { forward.x, forward.y, forward.z, up.x, up.y, up.z };
        alListenerfv(AL_ORIENTATION, orientation);

    }

    void AudioManager::UpdateSourcesAndTracks(Engine::GameEngine& game)
    {
        srcPool.ReclaimSources();

        for (auto& pair : activeTracks) pair.second.instance = nullptr;

        for (auto& pair : trackInstances)
        {
            int trackid = pair.second->GetTrackID();
            if (trackid < 0) continue;
            activeTracks.at(trackid).instance = pair.second;
        }

        for (auto it = activeTracks.begin(); it != activeTracks.end();)
        {
            it->second.Update(*this, game.deltaTime);
            if (it->second.toBeDeactivated) it = activeTracks.erase(it);
            else it++;
        }
    }

    int AudioManager::GetNextHandle()
    {
        return ++latestHandle;
    }

    void AudioManager::PlaceTrackInstance(int handle, TrackInstance* instance)
    {
        if (!instance) { trackInstances.erase(handle); return; }

        trackInstances.emplace(handle, instance);
        auto trackid = instance->GetTrackID();

        if (trackid > 0 && !activeTracks.contains(trackid)) activeTracks.emplace(trackid, *instance);
    }

    void AudioManager::DeactivateTrack(int trackid)
    {
        activeTracks.erase(trackid);
    }

    AudioManager::~AudioManager()
    {
        _sounds.clear();
        alcMakeContextCurrent(nullptr);
        if (audioContext) alcDestroyContext(audioContext);
        if (audioDevice) alcCloseDevice(audioDevice);
        std::cout << "[AudioManager] OpenAL Context Destroyed\n";
    }
}