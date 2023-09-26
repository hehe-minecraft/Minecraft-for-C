#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "constants.hpp"

class JsonList;
class JsonMap;
class Json;

class JsonList
{
	public:
		JsonList() = default;
		JsonList(const JsonList &);
		inline Json operator[](const int);
		operator bool();
		inline void append(const bool);
		inline void append(const int);
		inline void append(const double);
		inline void append(const std::wstring &);
		inline void append(const JsonList &);
		inline void append(const JsonMap &);
		inline void append(const Json &);
		unsigned int extend(const JsonList &);
		inline Json get(const int);
		inline unsigned int insert(const bool, const int);
		inline unsigned int insert(const int, const int);
		inline unsigned int insert(const double, const int);
		inline unsigned int insert(const std::wstring &, const int);
		inline unsigned int insert(const JsonList &, const int);
		inline unsigned int insert(const JsonMap &, const int);
		inline unsigned int insert(const Json &, const int);
		inline unsigned int length();
	protected:
		std::vector<Json> content;
};

class JsonMap
{
	public:
		JsonMap() = default;
		JsonMap(const JsonMap &);
		inline Json operator[](const std::wstring &);
		operator bool();
		inline void append(const std::wstring &, const bool);
		inline void append(const std::wstring &, const int);
		inline void append(const std::wstring &, const double);
		inline void append(const std::wstring &, const std::wstring &);
		inline void append(const std::wstring &, const JsonList &);
		inline void append(const std::wstring &, const JsonMap &);
		inline void append(const std::wstring &, const Json &);
		inline unsigned int append(const JsonMap &);
		inline unsigned int size();
	protected:
		std::unordered_map<std::wstring, Json> content;
};

class Json
{
	public:
		Json();
		Json(const Json &);
		Json(Json &&);
		Json(const bool);
		Json(const int);
		Json(const double);
		Json(const std::wstring &);
		Json(const JsonList &);
		Json(const JsonMap &);
		~Json();
		inline Json operator[](const int);
		inline Json operator[](const std::wstring &);
		void operator=(const Json &);
		operator bool() const;
		explicit operator bool();
		operator int() const;
		explicit operator int();
		operator double() const;
		explicit operator double();
		operator std::wstring() const;
		explicit operator std::wstring();
		operator JsonList() const;
		operator JsonMap() const;
	protected:
		choices::json type;
		void *content = nullptr;
};

namespace json_parser
{
	Json parse(std::wstring &, unsigned int &);
	Json parse_keyword(std::wstring &, unsigned int &);
	Json parse_string(std::wstring &, unsigned int &);
	Json parse_number(std::wstring &, unsigned int &);
	Json parse_list(std::wstring &, unsigned int &);
	Json parse_map(std::wstring &, unsigned int &);
};

Json parse(const std::wstring);
Json parse_string(std::wstring);
void dump(const std::wstring);
std::wstring dump_string();