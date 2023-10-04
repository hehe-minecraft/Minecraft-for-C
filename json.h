#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "constants.hpp"

class JsonList;
class JsonMap;
class Json;

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
		Json operator[](const int);
		Json operator[](const std::wstring &);
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

class JsonList
{
	public:
		JsonList() = default;
		JsonList(const JsonList &);
		Json& operator[](const int);
		operator bool();
		unsigned int extend(const JsonList &);
		unsigned int insert(const Json &item, const int index);
	protected:
		std::vector<Json> content;
	public:
		template <typename json_type>
		inline void append(const json_type &item)
		{
			this->append(Json(item));
		};
		inline void append(const Json &item)
		{
			this->content.push_back(item);
		};
		inline Json& get(const int index)
		{
			return this->operator[](index);
		};
		template <typename json_type>
		inline unsigned int insert(const json_type &item, const int index)
		{
			return this->insert(Json(item), index);
		};
		inline unsigned int length()
		{
			return this->content.size();
		};
};

class JsonMap
{
	public:
		JsonMap() = default;
		JsonMap(const JsonMap &);
		operator bool();
		unsigned int append(const JsonMap &);
	protected:
		std::unordered_map<std::wstring, Json> content;
	public:
		inline Json& operator[](const std::wstring &index)
		{
			return this->content[index];
		};
		template <typename json_type>
		inline void append(const std::wstring &key, const json_type &value)
		{
			this->append(key, Json(value));
		};
		inline void append(const std::wstring &key, const Json &value)
		{
			this->content[key] = value;
		};
		inline unsigned int size()
		{
			return this->content.size();
		};
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