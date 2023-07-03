#ifndef SETTINGS_STATE_H
#define SETTINGS_STATE_H

#include "vector"
#include "gaggia_settings.h"
#include "profiling_phases.h"
#include "saved_profiles.h"

namespace persistence {
  void init();

  const GaggiaSettings& getSettings();
  bool saveSettings(const GaggiaSettings& settings);

  const std::vector<SavedProfile>& getSavedProfiles();

  ProfileId getActiveProfileId();
  bool saveActiveProfileId(ProfileId id);

  bool profileExists(ProfileId id);
  std::pair<bool, Profile> getProfile(ProfileId id);
  bool deleteProfile(ProfileId id);
  bool saveProfile(ProfileId id, const Profile& profile);
  std::pair<bool, SavedProfile> saveNewProfile(const Profile& profile);

} // namespace state

#endif
