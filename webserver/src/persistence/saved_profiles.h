#ifndef PROFILE_SUMMARY_H
#define PROFILE_SUMMARY_H

#include "string"
#include "vector"

using ProfileId = uint32_t;

struct SavedProfile {
  ProfileId id;
  std::string name;
};

struct SavedProfiles {
  std::vector<SavedProfile> profiles;
};

#endif
