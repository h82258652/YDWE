#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include "BaseTable.hpp"
#include "ObjectId.hpp"
#include "VariableData.hpp"
#include "SlkTable.hpp"
#include "Util.hpp"
#include "MetaTable.hpp"
#include "Converter.hpp"

namespace slk
{
	enum OBJECT_PARSER_OPTION
	{
		ObjectWithOptinal,
		ObjectWithoutOptinal,
	};

	template <OBJECT_PARSER_OPTION Option>
	class Attribute;

	template <>
	class Attribute<ObjectWithOptinal>
	{
	public:
		ObjectId GetId() const
		{
			return _id;
		}

		void SetId(const ObjectId& id)
		{
			this->_id = id;
		}

		uint32_t GetLevel() const
		{
			return _level;
		}

		void SetLevel(uint32_t level)
		{
			this->_level = level;
		}

		uint32_t GetDataIndicator() const
		{
			return _dataIndicator;
		}

		void SetDataIndicator(uint32_t dataIndicator)
		{
			this->_dataIndicator = dataIndicator;
		}

		VariableData const& GetData() const
		{
			return _data;
		}

		void SetData(VariableData&& data)
		{
			this->_data = std::move(data);
		}

	protected:
		ObjectId         _id;
		uint32_t         _level;
		uint32_t         _dataIndicator;
		VariableData     _data;
	};

	template <>
	class Attribute<ObjectWithoutOptinal>
	{
	public:
		ObjectId GetId() const
		{
			return _id;
		}

		void SetId(const ObjectId& id)
		{
			this->_id = id;
		}

		VariableData const& GetData() const
		{
			return _data;
		}

		void SetData(VariableData const& data)
		{
			this->_data = data;
		}	

	protected:
		ObjectId         _id;
		VariableData     _data;
	};

	typedef HashTable<uint32_t, Attribute<ObjectWithOptinal>>::Type AttributeCatalog;

	template <OBJECT_PARSER_OPTION Option>
	class AttributeTable;

	template <>
	class AttributeTable<ObjectWithOptinal> : public BaseTable<AttributeTable<ObjectWithOptinal>, AttributeCatalog> { };

	template <>
	class AttributeTable<ObjectWithoutOptinal> : public BaseTable<AttributeTable<ObjectWithoutOptinal>, Attribute<ObjectWithoutOptinal>> { };

	template <OBJECT_PARSER_OPTION Option>
	class AttributeMeta
	{
	public:
		AttributeMeta(Attribute<Option> const& that, MetaTable const& metaTable, Converter& converter);
		void               Set(SlkSingle& obj) const;
		MetaSingle const*  GetMeta() const;

	private:
		bool               Init(Attribute<Option> const& that, MetaTable const& metaTable, Converter& converter);
		std::string        ToSlkName(Attribute<Option> const& that) const;

	private:
		bool valid_;
		std::string name_;
		std::string value_;
		MetaSingle const* meta_;
	};

	template <OBJECT_PARSER_OPTION Option>
	AttributeMeta<Option>::AttributeMeta(Attribute<Option> const& that, MetaTable const& metaTable, Converter& converter)
	{ 
		valid_ = Init(that, metaTable, converter);
	}

	template <OBJECT_PARSER_OPTION Option>
	bool AttributeMeta<Option>::Init(Attribute<Option> const& that, MetaTable const& metaTable, Converter& converter)
	{
		if (!metaTable.getValueById(that.GetId(), &meta_))
		{
			return false;
		}

		name_ = ToSlkName(that);
		VariableData const& data = that.GetData();
		if (data.GetType() != meta_->type)
			return false;

		value_ = data.ToString();

		if (meta_->stringExt)
		{
			value_ = converter.wts.Convert(value_);
		}

		return true;
	}

	template <OBJECT_PARSER_OPTION Option>
	void AttributeMeta<Option>::Set(SlkSingle& obj) const
	{
		if (valid_)
		{
			if (meta_->index == uint32_t(-1))
			{
				if (name_ == "auto" && meta_->slk == "UnitAbilities" && value_ == "")
				{
					obj[name_] = SlkValue("_", meta_->type == VariableData::OBJTYPE_STRING);
				}
				else
				{
					obj[name_] = SlkValue(value_, meta_->type == VariableData::OBJTYPE_STRING);
				}
				return ;
			}

			auto it = obj.find(name_);
			if (it == obj.end())
			{
				obj[name_] = SlkValue(value_, meta_->type == VariableData::OBJTYPE_STRING);
				return ;
			}

			SlkValue& val = it->second;
			std::vector<std::string> result;
			split(result, val.to_string(), char_equal(','));

			if (meta_->index >= result.size())
			{
				return ;
			}

			result[meta_->index] = value_;
			val.set(concat(result, ','));
		}
	}

	template <OBJECT_PARSER_OPTION Option>
	MetaSingle const*  AttributeMeta<Option>::GetMeta() const
	{
		if (!valid_) 
			return nullptr;

		return meta_;
	}
}
