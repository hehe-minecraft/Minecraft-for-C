#include <fstream>
#include <regex>
#include <stack>
#include "json.h"

static inline void no_null_ptr(void *pointer)
{
	if (pointer == nullptr)
	{
		throw errors::NullPointerError();
	};
};

static inline void skip_space(const std::wstring &string, unsigned int &iter)
{
	while (std::isspace(string[iter]))
	{
		iter++;
	};
};

JsonList::JsonList(const JsonList &item)
{
	this->extend(item);
};

inline Json JsonList::operator[](const int index)
{
	if (index >= this->length() || index < -this->length())
	{
		throw errors::IndexError();
	};
	if (index >= 0)
	{
		return this->content[index];
	}
	else
	{
		return this->content[this->length() + index]; // index is negative so we use adding.
	};
};

JsonList::operator bool()
{
	return this->length();
};

inline void JsonList::append(const bool item)
{
	this->append(Json(item));
};

inline void JsonList::append(const int item)
{
	this->append(Json(item));
};

inline void JsonList::append(const double item)
{
	this->append(Json(item));
};

inline void JsonList::append(const std::wstring &item)
{
	this->append(Json(item));
};

inline void JsonList::append(const JsonList &item)
{
	this->append(Json(item));
};

inline void JsonList::append(const JsonMap &item)
{
	this->append(Json(item));
};

inline void JsonList::append(const Json &item)
{
	this->content.push_back(item);
};

unsigned int JsonList::extend(const JsonList &item)
{
	for (auto each_item : item.content)
	{
		this->append(Json(each_item));
	};
	return this->length();
};

inline Json JsonList::get(const int index)
{
	return this[index];
};

inline unsigned int JsonList::insert(const bool item, const int index)
{
	return this->insert(Json(item), index);
};

inline unsigned int JsonList::insert(const int item, const int index)
{
	return this->insert(Json(item), index);
};

inline unsigned int JsonList::insert(const double item, const int index)
{
	return this->insert(Json(item), index);
};

inline unsigned int JsonList::insert(const std::wstring &item, const int index)
{
	return this->insert(Json(item), index);
};

inline unsigned int JsonList::insert(const JsonList &item, const int index)
{
	return this->insert(Json(item), index);
};

inline unsigned int JsonList::insert(const JsonMap &item, const int index)
{
	return this->insert(Json(item), index);
};

inline unsigned int JsonList::insert(const Json &item, const int index)
{
	if (index > this->length() || index < -this->length())
	{
		throw errors::IndexError();
	};
	if (index > 0)
	{
		this->content.insert(this->content.begin() + index, item);
	}
	else
	{
		this->content.insert(this->content.begin() + this->length() + index, item); // index is negative so we use adding.
	};
	return this->length();
};

inline unsigned int JsonList::length()
{
	return this->content.size();
};

JsonMap::JsonMap(const JsonMap &item)
{
	this->append(item);
};

inline Json JsonMap::operator[](const std::wstring &index)
{
	return this->content[index];
};

JsonMap::operator bool()
{
	return this->size();
}

inline void JsonMap::append(const std::wstring &key, const bool value)
{
	this->append(key, Json(value));
};

inline void JsonMap::append(const std::wstring &key, const int value)
{
	this->append(key, Json(value));
};

inline void JsonMap::append(const std::wstring &key, const double value)
{
	this->append(key, Json(value));
};

inline void JsonMap::append(const std::wstring &key, const std::wstring &value)
{
	this->append(key, Json(value));
};

inline void JsonMap::append(const std::wstring &key, const JsonList &value)
{
	this->append(key, Json(value));
};

inline void JsonMap::append(const std::wstring &key, const JsonMap &value)
{
	this->append(key, Json(value));
};

inline void JsonMap::append(const std::wstring &key, const Json &value)
{
	this->content.insert(std::make_pair(key, value));
};

inline unsigned int JsonMap::append(const JsonMap &source)
{
	for (auto &each_item : source.content)
	{
		this->append(std::wstring(each_item.first), Json(each_item.second));
	};
	return this->size();
};

inline unsigned int JsonMap::size()
{
	return this->content.size();
};

Json::Json()
{
	this->type = choices::json_null;
};

Json::Json(const Json &item)
{
	this->type = item.type;
	switch (item.type)
	{
		case choices::json_null:
			this->content = nullptr;
			break;
		case choices::json_bool:
			this->content = new bool((bool)item);
			break;
		case choices::json_int:
			this->content = new int((int)item);
			break;
		case choices::json_float:
			this->content = new double((double)item);
			break;
		case choices::json_string:
			this->content = new std::wstring((std::wstring)item);
			break;
		case choices::json_list:
			this->content = new JsonList((JsonList)item);
			break;
		case choices::json_map:
			this->content = new JsonMap((JsonMap)item);
			break;
		default:
			throw errors::UnexpectedControlError();
			break;
	};
};

Json::Json(Json &&item)
{
	this->type = item.type;
	this->content = item.content;
	item.content = nullptr;
};

Json::Json(const bool value)
{
	this->type = choices::json_bool;
	this->content = new bool(value);
};

Json::Json(const int value)
{
	this->type = choices::json_int;
	this->content = new int(value);
};

Json::Json(const double value)
{
	this->type = choices::json_float;
	this->content = new double(value);
};

Json::Json(const std::wstring &value)
{
	this->type = choices::json_string;
	this->content = new std::wstring(value);
};

Json::Json(const JsonList &value)
{
	this->type = choices::json_list;
	this->content = new JsonList(value);
};

Json::Json(const JsonMap &value)
{
	this->type = choices::json_map;
	this->content = new JsonMap(value);
};

Json::~Json()
{
	if (this->content != nullptr)
	{
		switch (this->type)
		{
			case choices::json_bool:
				delete (bool *)this->content;
				break;
			case choices::json_int:
				delete (int *)this->content;
				break;
			case choices::json_float:
				delete (double *)this->content;
				break;
			case choices::json_string:
				delete (std::wstring *)this->content;
				break;
			case choices::json_list:
				delete (JsonList *)this->content;
				break;
			case choices::json_map:
				delete (JsonMap *)this->content;
				break;
		};
		this->content = nullptr;
	};
};

void Json::operator=(const Json &item)
{
	if (this->content != nullptr)
	{
		switch (this->type)
		{
			case choices::json_bool:
				delete (bool *)this->content;
				break;
			case choices::json_int:
				delete (int *)this->content;
				break;
			case choices::json_float:
				delete (double *)this->content;
				break;
			case choices::json_string:
				delete (std::wstring *)this->content;
				break;
			case choices::json_list:
				delete (JsonList *)this->content;
				break;
			case choices::json_map:
				delete (JsonMap *)this->content;
				break;
		};
		this->content = nullptr;
	};
	this->type = item.type;
	switch (item.type)
	{
		case choices::json_null:
			this->content = nullptr;
			break;
		case choices::json_bool:
			this->content = new bool((bool)item);
			break;
		case choices::json_int:
			this->content = new int((int)item);
			break;
		case choices::json_float:
			this->content = new double((double)item);
			break;
		case choices::json_string:
			this->content = new std::wstring((std::wstring)item);
			break;
		case choices::json_list:
			this->content = new JsonList((JsonList)item);
			break;
		case choices::json_map:
			this->content = new JsonMap((JsonMap)item);
			break;
		default:
			throw errors::UnexpectedControlError();
			break;
	};
};

inline Json Json::operator[](const int index)
{
	if (this->type != choices::json_list)
	{
		throw errors::JsonTypeError();
	};
	return ((JsonList)*this)[index];
};

inline Json Json::operator[](const std::wstring &index)
{
	if (this->type != choices::json_map)
	{
		throw errors::JsonTypeError();
	};
	return ((JsonMap)*this)[index];
};

Json::operator bool() const
{
	no_null_ptr(this->content);
	if (this->type != choices::json_bool)
	{
		throw errors::JsonTypeError();
	};
	return *(bool*)this->content;
};

Json::operator bool()
{
	no_null_ptr(this->content);
	switch (this->type)
	{
		case choices::json_null:
			return false;
		case choices::json_bool:
			return *(bool*)this->content;
		case choices::json_int:
			return *(int*)this->content;
		case choices::json_float:
			return *(double*)this->content;
		case choices::json_string:
			return !((std::wstring*)this->content)->empty();
		case choices::json_list:
			return *(JsonList*)this->content;
		case choices::json_map:
			return *(JsonMap*)this->content;
		default:
			throw errors::UnexpectedControlError();
	};
};

Json::operator int() const
{
	no_null_ptr(this->content);
	if (this->type != choices::json_int)
	{
		throw errors::JsonTypeError();
	};
	return *(int*)this->content;
};

Json::operator int()
{
	no_null_ptr(this->content);
	switch (this->type)
	{
		case choices::json_null:
			return 0;
		case choices::json_bool:
			return *(bool*)this->content;
		case choices::json_int:
			return *(int*)this->content;
		case choices::json_float:
			return *(double*)this->content;
		case choices::json_string:
			try
			{
				return std::stoi(*(std::wstring*)this->content);
			}
			catch (std::invalid_argument)
			{
				throw errors::JsonTypeError();
			}
			catch (std::out_of_range)
			{
				throw errors::JsonTypeError();
			};
		default:
			throw errors::JsonTypeError(); // complex type (list, map)
	};
};

Json::operator double() const
{
	no_null_ptr(this->content);
	if (this->type != choices::json_float)
	{
		throw errors::JsonTypeError();
	};
	return *(double*)this->content;
};

Json::operator double()
{
	no_null_ptr(this->content);
	switch (this->type)
	{
		case choices::json_null:
			return 0.0;
		case choices::json_bool:
			return *(bool*)this->content;
		case choices::json_int:
			return *(int*)this->content;
		case choices::json_float:
			return *(double*)this->content;
		case choices::json_string:
			try
			{
				return std::stod(*(std::wstring*)this->content);
			}
			catch (std::invalid_argument)
			{
				throw errors::JsonTypeError();
			}
			catch (std::out_of_range)
			{
				throw errors::JsonTypeError();
			};
		default:
			throw errors::JsonTypeError(); // complex type (list, map)
	};
};

Json::operator std::wstring() const
{
	no_null_ptr(this->content);
	if (this->type != choices::json_string)
	{
		throw errors::JsonTypeError();
	};
	return *(std::wstring*)this->content;
};

Json::operator std::wstring()
{
	no_null_ptr(this->content);
	switch (this->type)
	{
		case choices::json_null:
			return std::wstring(L"null");
		case choices::json_bool:
			return std::to_wstring(*(bool*)this->content);
		case choices::json_int:
			return std::to_wstring(*(int*)this->content);
		case choices::json_float:
			return std::to_wstring(*(double*)this->content);
		case choices::json_string:
			return *(std::wstring*)this->content;
		default:
			throw errors::JsonTypeError(); // complex type (list, map)
	};
};

Json::operator JsonList() const
{
	no_null_ptr(this->content);
	if (this->type != choices::json_list)
	{
		throw errors::JsonTypeError();
	};
	return *(JsonList*)this->content;
};

Json::operator JsonMap() const
{
	no_null_ptr(this->content);
	if (this->type != choices::json_map)
	{
		throw errors::JsonTypeError();
	};
	return *(JsonMap*)this->content;
};

Json json_parser::parse(std::wstring &string, unsigned int &iter)
{
	skip_space(string, iter);
	switch (string[iter])
	{
		case '"':
			return parse_string(string, iter);
		case '[':
			return parse_list(string, iter);
		case '{':
			return parse_map(string, iter);
		default:
			if (std::isdigit(string[iter]) || string[iter] == '-')
			{
				return parse_number(string, iter);
			};
			return parse_keyword(string, iter);
	}
};

Json json_parser::parse_keyword(std::wstring &string, unsigned int &iter)
{
	std::wstring keyword_string = string.substr(iter, 5);
	if (keyword_string.starts_with(L"true"))
	{
		iter += 4;
		return Json(true);
	}
	else if (keyword_string.starts_with(L"false"))
	{
		iter += 5;
		return Json(false);
	}
	else if (keyword_string.starts_with(L"null"))
	{
		iter += 4;
		return Json();
	}
	else
	{
		throw errors::JsonSyntaxError();
	};
};

Json json_parser::parse_string(std::wstring &string, unsigned int &iter)
{
	if (string[iter] != '"')
	{
		throw errors::JsonSyntaxError();
	};
	std::wstring result_string;
	char current_char;
	while (iter < string.length())
	{
		iter += 1;
		current_char = string[iter];
		switch (current_char)
		{
			case '\\':
				iter += 1;
				current_char = string[iter];
				switch (current_char)
				{
					case 'b':
						result_string.push_back('\b');
						break;
					case 'f':
						result_string.push_back('\f');
						break;
					case 'n':
						result_string.push_back('\n');
						break;
					case 'r':
						result_string.push_back('\r');
						break;
					case 't':
						result_string.push_back('\t');
						break;
					case 'u': // such as "\u0000"
						result_string.push_back((wchar_t)std::stoi(string.substr(iter, 4)));
						iter += 4;
						break;
					case '"':
						result_string.push_back('"');
						break;
					case '/':
						result_string.push_back('/');
						break;
					case '\\':
						result_string.push_back('\\');
						break;
					default:
						throw errors::JsonBackSlashError();
				};
				break;
			case '"':
				iter++; // skip '"'
				return Json(result_string);
			default:
				result_string.push_back(current_char);
				break;
		};
	};
	throw errors::JsonEOFError();
};

static std::wregex number_regex(L"-?[0-9]+(\\.[0-9]*)?");
Json json_parser::parse_number(std::wstring &string, unsigned int &iter)
{
	std::wstring found_string = std::wsregex_iterator(string.begin() + iter, string.end(), number_regex)->str();
	iter += found_string.length();
	try
	{
		return Json(std::stoi(found_string));
	}
	catch (std::invalid_argument)
	{
		return Json(std::stod(found_string));
	};
};

Json json_parser::parse_list(std::wstring &string, unsigned int &iter)
{
	Json result(JsonList{});
	iter++; // skip '['
	while (true)
	{
		((JsonList)result).append(json_parser::parse(string, iter));
		skip_space(string, iter);
		if (string[iter] == ']')
		{
			break;
		}
		else if (string[iter] == ',')
		{
			iter++; // skip ','
		}
		else
		{
			throw errors::JsonSyntaxError();
		};
	};
	iter++; // skip ']'
	return result;
};

Json json_parser::parse_map(std::wstring &string, unsigned int &iter)
{
	Json result(JsonMap{});
	iter++; // skip '{'
	while (true)
	{
		skip_space(string, iter);
		if (string[iter] != '"')
		{
			throw errors::JsonSyntaxError();
		};
		Json key_json = json_parser::parse_string(string, iter);
		skip_space(string, iter);
		if (string[iter] == ':')
		{
			iter++;
		}
		else
		{
			throw errors::JsonSyntaxError();
		};
		Json value_json = json_parser::parse(string, iter);
		((JsonMap)result).append(key_json, value_json);
		skip_space(string, iter);
		if (string[iter] == '}')
		{
			break;
		}
		else if (string[iter] == ',')
		{
			iter++; // skip ','
		}
		else
		{
			throw errors::JsonSyntaxError();
		};
	};
	iter++; // skip '}'
	return result;
};

Json parse(const std::string file_name)
{
	typedef std::istreambuf_iterator<char> file_buffer;
	std::ifstream file(file_name);
	if (!file)
	{
		throw errors::FileNotFoundError();
	};
	return parse_string(std::wstring(file_buffer(file), file_buffer()));
};

Json parse_string(std::wstring source)
{
	unsigned int index = 0;
	return json_parser::parse(source, index);
};