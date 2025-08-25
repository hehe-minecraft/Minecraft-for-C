export module core.file;

import std;
import sfml;
import core.constant;

export namespace file
{
	std::string text_from_file(const std::filesystem::path& file_name)
	{
		std::ifstream file_stream;
		file_stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		file_stream.open(file_name, std::ios_base::in);
		std::stringstream string_stream;
		string_stream << file_stream.rdbuf();
		file_stream.close();
		return string_stream.str();
	};
	sf::Image image_from_file(const std::filesystem::path& file_name)
	{
		sf::Image image;
		const bool success = image.loadFromFile(file_name.string());
		if (not success)
		{
			throw errors::FileNotFoundError();
		};
		return image;
	};
};