
#pragma once

//
// ファイル関連の雑用
//

#include <sys/stat.h>
#include <string>


namespace {

// ディレクトリ名を返す
// ex) hoge/fuga/piyo.txt -> hoge/fuga
std::string getDirectoryname(const std::string& path) {
	std::string::size_type end(path.rfind('/'));
	return (end != std::string::npos) ? path.substr(0, end) : std::string();
}

// ファイル名を返す
// ex) hoge/fuga/piyo.txt -> piyo.txt
std::string getFilename(const std::string& path) {
	return path.substr(path.rfind('/') + 1, path.length());
}

// ディレクトリ名と拡張子を除いたファイル名を返す
// ex) hoge/fuga/piyo.txt -> piyo
std::string getFilenameNoExt(const std::string& path) {
	std::string name = getFilename(path);
	return name.substr(0, name.rfind('.'));
}

// パスから拡張子を返す
// ex) hoge/fuga/piyo.txt -> txt
std::string getFilenameExt(const std::string& path) {
	std::string::size_type pos(path.rfind('.'));
	return (pos != std::string::npos) ? path.substr(pos + 1, path.length()) : std::string();
}

// 拡張子を変更する
// ex) hoge/fuga/piyo.txt -> hoge/fuga/piyo.data
std::string replaceFilenameExt(const std::string& path, const std::string& ext) {
	std::string::size_type pos(path.rfind('.'));
  return (pos != std::string::npos) ? path.substr(0, path.rfind('.') + 1) + ext : path + ext;
}

// パスの有効判定
bool isValidPath(const std::string& path) {
	struct stat info;
	int result = stat(path.c_str(), &info);
	return (result == 0);
	// TODO: ディレクトリかどうかも判定
}


}
