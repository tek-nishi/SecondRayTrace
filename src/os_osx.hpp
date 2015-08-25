
#pragma once

//
// OS依存処理(OSX版)
//

#if defined (__APPLE__) && !(TARGET_OS_IPHONE)

#include "defines.hpp"
#include <unistd.h>
#include <iostream>
#include <string>
#include <boost/noncopyable.hpp>
#include <sys/stat.h>


namespace {
  
class Os : private boost::noncopyable {
	std::string resource_path_;
	std::string document_path_;

  
public:
	Os() {
		DOUT << "Os()" << std::endl;

		std::string path = currentPath();
		resource_path_ = path + "res/";
		document_path_ = path;
	}

	~Os() {
		DOUT << "~Os()" << std::endl;
	}

  
	const std::string& resourcePath() const { return resource_path_; }
	const std::string& documentPath() const { return document_path_; }

  
  static void createDirecrory(const std::string& path) {
    mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
  }


private:
#ifdef DEBUG

#ifndef SRCROOT
#define SRCROOT ./
#endif

  // デバッグ時はプロジェクトのディレクトリからファイルを読む
  static std::string currentPath() {
    return std::string(PREPRO_TO_STR(SRCROOT));
  }

#else

  // リリース時は.app内のファイルを読み込む
  static std::string currentPath() {
    return std::string("");
  }

#endif
};

}

#endif
