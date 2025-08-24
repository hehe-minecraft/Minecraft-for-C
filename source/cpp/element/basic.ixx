export module core.element;

import std;

class SerializeContext;

export namespace element
{
	class Element
	{
		protected:
			Element() = default;
			Element(const Element&) = delete;
			Element(Element&&) = default;
		public:
			virtual ~Element() = default;
			virtual void serialize(SerializeContext& context) const = 0;
	};
	using element_ptr = std::shared_ptr<Element>;
	class GameDataNull : public Element
	{
		public:
			GameDataNull() = default;
			GameDataNull(const GameDataNull&) = default;
			GameDataNull(GameDataNull&&) = default;
			void serialize(SerializeContext& context) const;
	};
	class GameDataBool : public Element
	{
		protected:
			bool value;
		public:
			GameDataBool() = delete;
			GameDataBool(const GameDataBool&) = default;
			GameDataBool(GameDataBool&&) = default;
			explicit GameDataBool(bool value) noexcept :
				value{ value }
			{};
			void serialize(SerializeContext&) const;
	};
	class GameDataFastInt : public Element
	{
		protected:
			std::int32_t value;
		public:
			GameDataFastInt() noexcept :
				value{ 0 }
			{};
			GameDataFastInt(const GameDataFastInt&) = default;
			GameDataFastInt(GameDataFastInt&&) = default;
			explicit GameDataFastInt(std::int32_t value) noexcept :
				value{ value }
			{};
			void serialize(SerializeContext& context) const;
	};
	class GameDataFloat : public Element
	{
		protected:
			double value;
		public:
			GameDataFloat() noexcept :
				value{ 0 }
			{};
			GameDataFloat(const GameDataFloat&) = default;
			GameDataFloat(GameDataFloat&&) = default;
			explicit GameDataFloat(double value) noexcept :
				value{ value }
			{};
			void serialize(SerializeContext& context) const;
	};
	class GameDataString : public Element
	{
		protected:
			std::string value;
		public:
			GameDataString() = default;
			GameDataString(const GameDataString&) = default;
			GameDataString(GameDataString&&) = default;
			explicit GameDataString(const std::string& value) noexcept :
				value{ value }
			{};
			void serialize(SerializeContext& context) const;
	};
	class GameDataList : public Element
	{
		protected:
			std::vector<element_ptr> items;
		public:
			GameDataList() = default;
			GameDataList(const GameDataList&) = default;
			GameDataList(GameDataList&&) = default;
			explicit GameDataList(const std::vector<element_ptr>& items) noexcept :
				items{ items }
			{};
			void append(element_ptr value)
			{
				this->items.push_back(value);
			};
			void serialize(SerializeContext& context) const;
	};
	class GameDataSet : public Element
	{
		protected:
			std::set<element_ptr> items;
		public:
			GameDataSet() = default;
			GameDataSet(const GameDataSet&) = default;
			GameDataSet(GameDataSet&&) = default;
			explicit GameDataSet(const std::set<element_ptr>& items) noexcept :
				items{ items }
			{};
			void append(element_ptr value)
			{
				this->items.insert(value);
			};
			void serialize(SerializeContext& context) const;
	};
	class GameDataDict : public Element
	{
		protected:
			std::map<element_ptr, element_ptr> items;
		public:
			GameDataDict() = default;
			GameDataDict(const GameDataDict&) = default;
			GameDataDict(GameDataDict&&) = default;
			explicit GameDataDict(const std::map<element_ptr, element_ptr>& items) noexcept :
				items{ items }
			{};
			void append(element_ptr key, element_ptr value)
			{
				this->items.insert({ key, value });
			};
			void serialize(SerializeContext& context) const;
	};
};

export namespace serialization
{
	using byte = std::uint8_t; // Standard std::byte requires too many casts.
	using serial = std::vector<byte>;
	serial serialize(const element::element_ptr& object);
	element::element_ptr deserialize(std::span<byte> serial);
};