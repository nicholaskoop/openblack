/*****************************************************************************
 * Copyright (c) 2018-2020 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "SoundHandler.h"
#include "3D/Camera.h"
#include "Game.h"
#include "Math.h"

#include <fstream>
#include <vector>

namespace openblack::audio
{

SoundHandler::SoundHandler(std::unique_ptr<AudioDecoder>&& decoder, std::unique_ptr<AudioPlayer>&& player)
    : _audioDecoder(std::move(decoder))
    , _audioPlayer(std::move(player))
{
	_emitterId = 0;
	auto emptyVec = glm::zero<glm::vec3>();
	_audioPlayer->UpdateListenerState(glm::one<glm::vec3>(), emptyVec, emptyVec, emptyVec);
	_audioPlayer->SetVolume(.5f);
}

void SoundHandler::RegisterSoundPack(std::unique_ptr<SoundPack>& soundPack)
{
	auto& name = soundPack->GetFileName();
	spdlog::debug("Registering SoundPack {}", name);

	if (_soundPackLookup.find(name) != _soundPackLookup.end())
	{
		throw std::runtime_error("SoundPack " + name + " already registered");
	}

	_soundPackLookup[name] = std::move(soundPack);
	auto& sounds = _soundPackLookup[name]->GetSounds();

	for (auto& [id, sound] : sounds)
	{
		if (_soundIdLookup.find(id) == _soundIdLookup.end())
			_soundIdLookup[id] = _soundPackLookup[name];
		else
			spdlog::warn("Duplicate sound ID (\"{}\"). Unable to add sound", id);
	}
}

void SoundHandler::Tick(Game& game)
{
	// Update listener state
	auto& cam = game.GetCamera();
	auto pos = cam.GetPosition();
	auto vel = cam.GetVelocity();
	auto forward = cam.GetForward();
	auto top = cam.GetUp();
	_audioPlayer->UpdateListenerState(pos, vel, forward, top);

	// Update active user emitter states
	for (auto& [emitterId, emitter] : _emitters) _audioPlayer->UpdateEmitterState(emitter);

	// Sort activated emitters
	// TODO: Implement

	// Release hardware resources
	// Update emitters with hardware resources
	// Assign hardware resources
	//	for (auto& [id, emitter] : _emitters)
	//	{
	//		_audioPlayer->CleanUpResource(emitter);
	//	}

	// Cull stopped emitters
	for (auto iterator = _emitters.begin(); iterator != _emitters.end();)
	{
		auto& [id, emitter] = *iterator;
		auto audioStatus = _audioPlayer->GetAudioStatus(emitter.audioSourceId);

		if (emitter.loop == PlayType::Once && audioStatus == AudioStatus::Stopped)
		{
			_audioPlayer->CleanUpResource(emitter);
			iterator = _emitters.erase(iterator);
		}
		else
			iterator++;
	}
}

std::unique_ptr<Sound>& SoundHandler::GetSound(std::string soundPackName, std::string soundName)
{
	return _soundPackLookup[soundPackName]->GetSoundId(soundName);
}

std::unique_ptr<Sound>& SoundHandler::GetSound(SoundId id)
{
	auto result = _soundIdLookup.find(id);

	if (result == _soundIdLookup.end())
		throw SoundNotFoundError(id);

	auto& sound = _soundIdLookup[id]->GetSound(id);

	if (!sound->loaded)
	{
		try
		{
			_audioDecoder->ToPCM16(*sound);
			sound->loaded = true;
		} catch (std::runtime_error& err)
		{
			spdlog::error("Failed to decode sound: {}", err.what());
		}
	}

	return sound;
}

const std::map<AudioEmitterId, AudioEmitter>& SoundHandler::GetEmitters() const
{
	return _emitters;
}

AudioEmitterId SoundHandler::CreateEmitters(const std::vector<SoundId> ids, glm::vec3& position, glm::vec3& velocity,
                                            glm::vec3& direction, glm::vec2& radius, float volume, bool world,
                                            PlayType playType)
{
	std::vector<SoundId> out;
	std::sample(ids.begin(), ids.end(), std::back_inserter(out), 1, Math::GetRandom());
	return CreateEmitter(out[0], position, velocity, direction, radius, volume, world, playType);
}

AudioEmitterId SoundHandler::CreateEmitter(SoundId id, glm::vec3& position, glm::vec3& velocity, glm::vec3& direction,
                                           glm::vec2& radius, float volume, bool world, PlayType playType)
{
	auto& sound = GetSound(id);
	AudioEmitterId emitterId = _emitterId++;
	volume = sound->volume * volume;
	AudioEmitter emitter = {0, 0, id, 0, position, velocity, direction, radius, volume, playType, world};
	_audioPlayer->SetupEmitter(emitter, *sound);
	_emitters[emitterId] = emitter;
	return emitterId;
}

bool SoundHandler::EmitterExists(AudioEmitterId id)
{
	return _emitters.find(id) != _emitters.end();
}

void SoundHandler::PlaySound(SoundId id, PlayType playType)
{
	auto unusedPosVel = glm::vec3(1);
	auto unusedRadius = glm::vec2(1);
	auto emitterId = CreateEmitter(id, unusedPosVel, unusedPosVel, unusedPosVel, unusedRadius, 1.0, false, playType);
	_audioPlayer->PlayEmitter(_emitters[emitterId]);
}

void SoundHandler::PlayEmitter(AudioEmitterId id)
{
	_audioPlayer->PlayEmitter(_emitters[id]);
}

void SoundHandler::PauseEmitter(AudioEmitterId id)
{
	_audioPlayer->PauseEmitter(_emitters[id]);
}

void SoundHandler::StopEmitter(AudioEmitterId id)
{
	_audioPlayer->StopEmitter(_emitters[id]);
}

void SoundHandler::DestroyEmitters()
{
	for (auto iterator = _emitters.begin(); iterator != _emitters.end();)
	{
		auto& [id, emitter] = *iterator;
		_audioPlayer->CleanUpResource(emitter);
		iterator = _emitters.erase(iterator);
	}
}

void SoundHandler::DestroyEmitter(AudioEmitterId id)
{
	_audioPlayer->CleanUpResource(_emitters[id]);
	_emitters.erase(id);
}

void SoundHandler::SetGlobalVolume(float volume)
{
	_audioPlayer->SetVolume(volume);
}

void SoundHandler::ReplaceAudioPlayer(std::unique_ptr<AudioPlayer> audioPlayer)
{
	// Clean up everything
	DestroyEmitters();
	// Replace the old player with the new one
	_audioPlayer.reset(audioPlayer.release());
	_audioPlayer->SetVolume(.5f);
	// Activate the new player now that the old is destroyed
	_audioPlayer->Activate();
}

std::unique_ptr<SoundHandler> CreateSoundHandler() noexcept
{
	// TODO: Temporary function for building the handler
	auto converter = std::make_unique<FfmpegDecoder>();
	auto player = std::make_unique<OpenAlPlayer>();
	player->Activate();
	return std::make_unique<SoundHandler>(std::move(converter), std::move(player));
}
} // namespace openblack::audio