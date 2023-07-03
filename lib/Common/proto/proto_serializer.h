#ifndef PROTO_SERIALIZER_H
#define PROTO_SERIALIZER_H

#include <vector>
#include "profiling_phases.h"
#include "nanopb_cpp.h"

/**
  * These are 2 utility function to make it easier to use NanoPB in our project where we use
  * Vectors to transmit or persist the serialized data (whereas NanoPB was designed to work with streams)
  *
  * The below two methods (serialize and deserialize) are just syntact sugar that serialize to/from a vector
  * instead of forcing us to use NanoPb::StringInputStream / NanoPb::StringOutputStream
  */
namespace ProtoSerializer {
  template<class MESSAGE_CONVERTER>
  std::vector<uint8_t> serialize(const typename MESSAGE_CONVERTER::LocalType& source) {
    NanoPb::StringOutputStream outputStream;
    bool check = NanoPb::encode<MESSAGE_CONVERTER>(outputStream, source);
    if (!check) {
      return std::vector<uint8_t>();
    }
    NanoPb::BufferPtr str = outputStream.release();

    std::vector<uint8_t> result(str->begin(), str->end());
    return result;
  }

  template<class MESSAGE_CONVERTER>
  bool deserialize(std::vector<uint8_t>& data, typename MESSAGE_CONVERTER::LocalType& target) {
    // Invoke the decoding
    bool result = NanoPb::decode<MESSAGE_CONVERTER>(data.data(), data.size(), target);
    return result;
  }
}

#endif
