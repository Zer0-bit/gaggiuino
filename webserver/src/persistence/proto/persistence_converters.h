#ifndef WEB_PERSISTENCE_CONVERTERS_H
#define WEB_PERSISTENCE_CONVERTERS_H

#include "nanopb_cpp.h"
#include "../saved_profiles.h"
#include "persistence.pb.h"


class SavedProfileConverter : public NanoPb::Converter::MessageConverter<SavedProfileConverter, SavedProfile, SavedProfileDto, SavedProfileDto_fields> {
public:
  // using PhaseArrayConverter = NanoPb::Converter::ArrayConverter<PhaseConverter, std::vector<Phase>>;

  static ProtoType encoderInit(const LocalType& local) {
    return SavedProfileDto{
      .id = local.id,
      .name = NanoPb::Converter::StringConverter::encoderInit(local.name),
    };
  };

  static ProtoType decoderInit(LocalType& local) {
    return SavedProfileDto{
      .name = NanoPb::Converter::StringConverter::decoderInit(local.name),
    };
  };

  static bool decoderApply(const ProtoType& proto, LocalType& local) {
    local.id = proto.id;
    return true;
  };
};

class SavedProfilesConverter : public NanoPb::Converter::MessageConverter<SavedProfilesConverter, SavedProfiles, SavedProfilesDto, SavedProfilesDto_fields> {
public:
  using SavedProfileArrayConverter = NanoPb::Converter::ArrayConverter<SavedProfileConverter, std::vector<SavedProfile>>;

  static ProtoType encoderInit(const LocalType& local) {
    return SavedProfilesDto{
      .profiles = SavedProfileArrayConverter::encoderCallbackInit(local.profiles),
    };
  };

  static ProtoType decoderInit(LocalType& local) {
    return SavedProfilesDto{
      .profiles = SavedProfileArrayConverter::decoderCallbackInit(local.profiles),
    };
  };

  static bool decoderApply(const ProtoType& proto, LocalType& local) {
    return true;
  };
};

#endif
