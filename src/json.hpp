
#pragma once

//
// Json
//

#include "defines.hpp"
#include <fstream>
#include <picojson.h>


namespace Json {

picojson::value read(const std::string& path) {
  std::ifstream fstr(path);
  if (!fstr) {
    DOUT << "File open error:" << path << std::endl;
    throw;
  }

  // ファイルからjsonオブジェクトを生成
  picojson::value json;
  fstr >> json;
  return json;
}

}
