#include "persistence.h"

#include "string"
#include "Preferences.h"
#include "proto/proto_serializer.h"
#include "proto/persistence_converters.h"
#include "proto/settings_converters.h"
#include "proto/profile_converters.h"
#include "default_settings.h"
#include "saved_profiles.h"
#include "../log/log.h"

#define KEY_PROFILE(id) std::string("p_id_" + std::to_string(id))

namespace persistence {
  SemaphoreHandle_t persistenceLock = xSemaphoreCreateRecursiveMutex();

  GaggiaSettings persistedSettings;
  SavedProfiles profileDictionary;
  ProfileId nextProfileId;
  ProfileId activeProfileId;

  const std::string KEY_NEXT_PROFILE_ID = "n_p_id";
  const std::string KEY_NAMESPACE = "gaggiuino_state";
  const std::string KEY_ACTIVE_PROFILE_ID = "act_p_id";
  const std::string KEY_SETTINGS = "g_settings";
  const std::string KEY_SAVED_PROFILES = "p_sum";

  Preferences preferences;

  // Private helper method definitions
  void initProfiles();
  void initSettings();
  void initNextProfileId();
  void initActiveProfileId();
  ProfileId getAndIncrementProfileId();
  std::vector<SavedProfile>::iterator findProfileById(ProfileId id_to_find);
  template<class MESSAGE_CONVERTER> void loadProtoResource(std::string key, typename MESSAGE_CONVERTER::LocalType& target);
  template<class MESSAGE_CONVERTER> void saveProtoResource(std::string key, const typename MESSAGE_CONVERTER::LocalType& source);

  // ----------------------------------------------------------------------------------------
  // --------------------------------- PUBLIC METHODS----------------------------------------
  // ----------------------------------------------------------------------------------------

  // Initialise settings or load defaults
  void init() {
    preferences.begin(KEY_NAMESPACE.c_str(), false);

    initSettings();
    LOG_INFO("Initialized settings");

    initNextProfileId();
    LOG_INFO("Initialized nextProfileId to %d", nextProfileId);

    initProfiles();
    LOG_INFO("Initialized profile dictionary [size=%d]", profileDictionary.profiles.size());

    initActiveProfileId();
    LOG_INFO("Initialized activeProfileId to %d", activeProfileId);
  }

  // Settings methods
  bool saveSettings(const GaggiaSettings& settings) {
    saveProtoResource<GaggiaSettingsConverter>(KEY_SETTINGS, settings);
    persistedSettings = settings;
    return true;
  }

  // Save gaggia settings
  const GaggiaSettings& getSettings() {
    return persistedSettings;
  }

  // Returns the list of known profiles with their names/ids. To get the full profile information
  // use getActiveProfile() or getProfile(id)
  const std::vector<SavedProfile>& getSavedProfiles() {
    return profileDictionary.profiles;
  }

  bool profileExists(ProfileId id) {
    auto profilePointer = findProfileById(id);
    return profilePointer != profileDictionary.profiles.end() && preferences.isKey(KEY_PROFILE(id).c_str());
  }

  // Get a profile by a given id
  std::pair<bool, Profile> getProfile(ProfileId id) {
    auto profilePointer = findProfileById(id);

    if (profilePointer == profileDictionary.profiles.end() || !preferences.isKey(KEY_PROFILE(id).c_str())) { // couldn't find profile, perform cleanup and return
      deleteProfile(id);
      return { false, {} };
    }

    Profile profile;
    loadProtoResource<ProfileConverter>(KEY_PROFILE(id), profile);
    return { true, profile };
  }

  ProfileId getActiveProfileId() {
    return activeProfileId;
  }

  bool saveActiveProfileId(ProfileId id) {
    if (xSemaphoreTakeRecursive(persistenceLock, portMAX_DELAY) == pdFALSE) return false;

    auto profilePointer = findProfileById(id);
    if (profilePointer == profileDictionary.profiles.end()) { // the id does not exist
      return false;
    }

    activeProfileId = id;
    preferences.putUInt(KEY_ACTIVE_PROFILE_ID.c_str(), activeProfileId);

    xSemaphoreGiveRecursive(persistenceLock);
    return true;
  }

  // Deletes profile with given id.
  bool deleteProfile(ProfileId id) {
    if (xSemaphoreTakeRecursive(persistenceLock, portMAX_DELAY) == pdFALSE) return false;

    auto profilePointer = findProfileById(id);
    if (profilePointer != profileDictionary.profiles.end()) { // the exists
      profileDictionary.profiles.erase(profilePointer); // remove it from the dictionary
      saveProtoResource<SavedProfilesConverter>(KEY_SAVED_PROFILES, profileDictionary);
    }

    if (preferences.isKey(KEY_PROFILE(id).c_str())) {
      preferences.remove(KEY_PROFILE(id).c_str()); // remove it from the storage
    }

    xSemaphoreGiveRecursive(persistenceLock);
    return true;
  }

  // Saves profile with a given id. If the profile cannot be found under this id, it will be created.
  bool saveProfile(ProfileId id, const Profile& profile) {
    if (xSemaphoreTakeRecursive(persistenceLock, portMAX_DELAY) == pdFALSE) return false;

    auto profilePointer = findProfileById(id);

    if (profilePointer == profileDictionary.profiles.end()) { // the id does not exist
      profileDictionary.profiles.push_back(SavedProfile{ .id = id, .name = profile.name });
    }

    if (profilePointer->name != profile.name) { // if name changed update the dictionary
      profilePointer->name = profile.name;
    }

    saveProtoResource<SavedProfilesConverter>(KEY_SAVED_PROFILES, profileDictionary);
    saveProtoResource<ProfileConverter>(KEY_PROFILE(id), profile);

    xSemaphoreGiveRecursive(persistenceLock);
    return true;
  }

  // Creates a new profile and returns its id.
  std::pair<bool, SavedProfile> saveNewProfile(const Profile& profile) {
    if (xSemaphoreTakeRecursive(persistenceLock, portMAX_DELAY) == pdFALSE) return { false, {} };

    SavedProfile newSavedProfile = SavedProfile{ .id = getAndIncrementProfileId(), .name = profile.name };

    if (!saveProfile(newSavedProfile.id, profile)) {
      xSemaphoreGiveRecursive(persistenceLock);
      return { false, {} };
    }

    xSemaphoreGiveRecursive(persistenceLock);
    return { true, newSavedProfile };
  }

  // ----------------------------------------------------------------------------------------
  // ----------------------------- PRIVATE HELPER METHODS -----------------------------------
  // ----------------------------------------------------------------------------------------

  // Initializes profile variables (dictionary and saved profile resources)
  void initProfiles() {
    // Retrieve profile summaries or initialize profiles
    if (preferences.isKey(KEY_SAVED_PROFILES.c_str())) {
      LOG_INFO("Found profile dictionary");
      loadProtoResource<SavedProfilesConverter>(KEY_SAVED_PROFILES, profileDictionary);
      // TODO: Should we perform health check of saved profiles and attempt to recover?
      return;
    }

    // If saved profiles were not present initialize all resources to defaults
    LOG_INFO("Not found profile dictionary");
    std::vector<Profile> profiles = default_settings::getDefaultProfiles();
    profileDictionary.profiles.clear();

    for (auto& profile : profiles) {
      SavedProfile savedProfile = SavedProfile{ .id = nextProfileId, .name = profile.name };
      profileDictionary.profiles.push_back(savedProfile);
      saveProtoResource<ProfileConverter>(KEY_PROFILE(savedProfile.id), profile);
      nextProfileId += 1;
    }

    saveProtoResource<SavedProfilesConverter>(KEY_SAVED_PROFILES, profileDictionary);
    preferences.putUInt(KEY_NEXT_PROFILE_ID.c_str(), nextProfileId);
  }

  // Retrieve settings or initialize to defaults if not present
  void initSettings() {
    if (preferences.isKey(KEY_SETTINGS.c_str())) {
      LOG_INFO("Found persisted settings. Will load them");
      GaggiaSettings settings;
      loadProtoResource<GaggiaSettingsConverter>(KEY_SETTINGS, settings);
      LOG_INFO("Default boiler.hpwr = %d", settings.boiler.hpwr);
      persistedSettings = settings;
      return;
    }

    LOG_INFO("Not found persisted settings. Will load defaults");
    saveSettings(default_settings::getDefaultSettings());
    LOG_INFO("Default boiler.hpwr = %d", persistedSettings.boiler.hpwr);
    loadProtoResource<GaggiaSettingsConverter>(KEY_SETTINGS, persistedSettings);
    LOG_INFO("Reloaded boiler.hpwr = %d", persistedSettings.boiler.hpwr);
  }

  // Initialize nextProfileId
  void initNextProfileId() {
    if (preferences.isKey(KEY_NEXT_PROFILE_ID.c_str())) {
      LOG_INFO("Found nextProfileId. Will load it");
      nextProfileId = preferences.getUInt(KEY_NEXT_PROFILE_ID.c_str());
      return;
    }

    LOG_INFO("Not nextProfileId. Will default it");
    nextProfileId = 0u;
    preferences.putUInt(KEY_NEXT_PROFILE_ID.c_str(), nextProfileId);
  }

  // Retrieve active profile index or initialize to zero
  void initActiveProfileId() {
    if (preferences.isKey(KEY_ACTIVE_PROFILE_ID.c_str())) {
      LOG_INFO("Found activeProfileId. Will load it");
      activeProfileId = preferences.getUInt(KEY_ACTIVE_PROFILE_ID.c_str());
    }
    else {
      LOG_INFO("Not found activeProfileId. Will default it");
      activeProfileId = 0u;
      preferences.putUInt(KEY_ACTIVE_PROFILE_ID.c_str(), activeProfileId);
    }
  }

  ProfileId getAndIncrementProfileId() {
    ProfileId result = nextProfileId++;
    preferences.putUInt(KEY_NEXT_PROFILE_ID.c_str(), nextProfileId);
    return result;
  }

  // Searches in the profile dictionary for a profile with the given id and returns
  // an iterator pointer to the element of that id, or a pointer to the end of the dictionary
  // if the id was not found
  std::vector<SavedProfile>::iterator findProfileById(ProfileId id) {
    for (auto it = profileDictionary.profiles.begin(); it != profileDictionary.profiles.end(); ++it) {
      if (it->id == id) {
        return it;
      }
    }
    return profileDictionary.profiles.end();
  }

  template<class MESSAGE_CONVERTER>
  void loadProtoResource(std::string key, typename MESSAGE_CONVERTER::LocalType& target) {
    size_t size = preferences.getBytesLength(key.c_str());
    std::vector<uint8_t> data(size);
    LOG_INFO("Retrieving key[%s] from nvs with size=%dbytes", key.c_str(), size);
    preferences.getBytes(key.c_str(), data.data(), size);
    ProtoSerializer::deserialize<MESSAGE_CONVERTER>(data, target);
  }

  template<class MESSAGE_CONVERTER>
  void saveProtoResource(std::string key, const typename MESSAGE_CONVERTER::LocalType& source) {
    std::vector<uint8_t> data = ProtoSerializer::serialize<MESSAGE_CONVERTER>(source);
    LOG_INFO("Saving key[%s] to nvs with size=%dbytes", key.c_str(), data.size());
    preferences.putBytes(key.c_str(), data.data(), data.size());
  }
} // namespace state
