/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#include "Asset/Music.h"
#include <cassert>
#include <iostream>

namespace Shiro {
    MusicAssetLoader::MusicAssetLoader(const std::filesystem::path& basePath) :
        basePath(basePath) {}

    std::unique_ptr<Asset> MusicAssetLoader::create(const std::filesystem::path& location) const {
        return std::unique_ptr<Shiro::Asset>(new MusicAsset(location));
    }

    bool MusicAssetLoader::load(Asset& asset) const {
        assert(asset.getType() == AssetType::music);
        MusicAsset& musicAsset = static_cast<MusicAsset&>(asset);

        musicAsset.data = Mix_LoadMUS((basePath / asset.location).concat(".ogg").string().c_str());
        if (musicAsset.data) {
            musicAsset.volume = 100.0f;
            return true;
        }

        musicAsset.data = Mix_LoadMUS((basePath / asset.location).concat(".wav").string().c_str());
        if (musicAsset.data) {
            musicAsset.volume = 100.0f;
            return true;
        }

        std::cerr << "Failed loading music \"" << musicAsset.location.string() << "\"" << std::endl;
        musicAsset.volume = 0.0f;
        return false;
    }

    void MusicAssetLoader::unload(Asset& asset) const {
        assert(asset.getType() == AssetType::music);
        MusicAsset& musicAsset = static_cast<MusicAsset&>(asset);

        if (musicAsset.loaded()) {
            Mix_FreeMusic(musicAsset.data);
            musicAsset.data = nullptr;
        }
    }

    AssetType MusicAssetLoader::getType() const {
        return AssetType::music;
    }

    MusicAsset::MusicAsset(const std::filesystem::path& location) :
        Asset(location),
        volume(0.0f),
        data(nullptr) {}

    MusicAsset::~MusicAsset() {}

    bool MusicAsset::play(const Settings& settings) const {
        if (!loaded()) {
            return false;
        }

        Mix_VolumeMusic(static_cast<int>(MIX_MAX_VOLUME * (volume / 100.0f) * (settings.musicVolume / 100.0f) * (settings.masterVolume / 100.0f)));
        Mix_PlayMusic(data, -1);
        return true;
    }

    bool MusicAsset::loaded() const {
        return data != nullptr;
    }

    AssetType MusicAsset::getType() const {
        return AssetType::music;
    }
}