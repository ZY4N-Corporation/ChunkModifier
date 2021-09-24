#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <functional>

#include <BEstream.hpp>


class NBT;

using NBTstring = std::string;
using NBTbyteArray = std::vector<int8_t>;
using NBTintArray = std::vector<int32_t>;
using NBTlongArray = std::vector<int64_t>;
using NBTlist = std::vector<NBT>;
using NBTcompound = std::unordered_map<std::string, NBT>;

union NBTtag {
	bool Boolean;
	int8_t Byte;
	int16_t Short;
	int32_t Int;
	int64_t Long;
	float Float;
	double Double;
	NBTstring* String;
	NBTbyteArray* ByteArray;
	NBTintArray* IntArray;
	NBTlongArray* LongArray;
	NBTlist* List;
	NBTcompound* Compound;
};

enum class NBTtagType : uint8_t {
	end = 0,
	Byte = 1,
	Short = 2,
	Int = 3,
	Long = 4,
	Float = 5,
	Double = 6,
	ByteArray = 7,
	String = 8,
	List = 9,
	Compound = 10,
	IntArray = 11,
	LongArray = 12,
	Boolean = 13,
	null = UINT8_MAX
};

class NBT {
private:
	NBTtag data;
	NBTtagType type;

	void cleanup();

	static const constexpr char tab = '\t';

	static const auto getParser(NBTtagType);

	static NBT parseBool(BEstream&);
	static NBT parseByte(BEstream&);
	static NBT parseShort(BEstream&);
	static NBT parseInt(BEstream&);
	static NBT parseLong(BEstream&);
	static NBT parseString(BEstream&);
	static NBT parseByteArray(BEstream&);
	static NBT parseIntArray(BEstream&);
	static NBT parseLongArray(BEstream&);
	static NBT parseList(BEstream&);
	static NBT parseCompound(BEstream&);

	void serialize(BEstream&) const;

	size_t size(bool firstCall = true) const;

public:

	NBT();
	NBT(const NBT&);
	NBT(NBT&&) noexcept;

	NBT(bool);
	NBT(int8_t);
	NBT(int16_t);
	NBT(int32_t);
	NBT(int64_t);
	NBT(float);
	NBT(double);
	NBT(const NBTstring&);
	NBT(const NBTbyteArray&);
	NBT(const NBTintArray&);
	NBT(const NBTlongArray&);
	NBT(const NBTlist&);
	NBT(const NBTcompound&);

	NBT(NBTstring&&);
	NBT(NBTbyteArray&&);
	NBT(NBTintArray&&);
	NBT(NBTlongArray&&);
	NBT(NBTlist&&);
	NBT(NBTcompound&&);

	~NBT();

	static std::string typeToString(NBTtagType);

	static NBT parse(uint8_t* buffer, size_t size);

	size_t length();

	uint8_t* serialize(size_t& size) const;

	void toString(std::stringstream& out, int indent = -1) const;

	NBTtagType getType() const { return type; };


	explicit operator bool() const;
	explicit operator int8_t() const;
	explicit operator int16_t() const;
	explicit operator int32_t() const;
	explicit operator int64_t() const;
	explicit operator float() const;
	explicit operator double() const;

	operator const NBTstring& () const;
	operator const NBTbyteArray& () const;
	operator const NBTintArray& () const;
	operator const NBTlongArray& () const;
	operator const NBTlist& () const;
	operator const NBTcompound& () const;

	explicit operator bool& ();
	explicit operator int8_t& ();
	explicit operator int16_t& ();
	explicit operator int32_t& ();
	explicit operator int64_t& ();
	explicit operator float& ();
	explicit operator double& ();

	operator NBTstring& ();
	operator NBTbyteArray& ();
	operator NBTintArray& ();
	operator NBTlongArray& ();
	operator NBTlist& ();
	operator NBTcompound& ();

	NBT& operator=(const NBT&);
	NBT& operator=(NBT&&) noexcept;

	NBT& operator=(bool);
	NBT& operator=(int8_t);
	NBT& operator=(int16_t);
	NBT& operator=(int32_t);
	NBT& operator=(int64_t);
	NBT& operator=(float);
	NBT& operator=(double);
	NBT& operator=(const NBTstring&);
	NBT& operator=(const NBTbyteArray&);
	NBT& operator=(const NBTintArray&);
	NBT& operator=(const NBTlongArray&);
	NBT& operator=(const NBTlist&);
	NBT& operator=(const NBTcompound&);

	NBT& operator=(NBTstring&&);
	NBT& operator=(NBTbyteArray&&);
	NBT& operator=(NBTintArray&&);
	NBT& operator=(NBTlongArray&&);
	NBT& operator=(NBTlist&&);
	NBT& operator=(NBTcompound&&);

	NBT& operator[](const size_t);
	NBT& operator[](const std::string&);

	template<typename T>
	T& at(const size_t index) {
		if (type == NBTtagType::List) {
			return static_cast<T&>(data.List->at(index));
		}
		throw std::bad_cast();
	}

	template<typename T>
	T& at(const std::string& key) {
		if (type == NBTtagType::Compound) {
			return static_cast<T&>(data.Compound->at(key));
		}
		throw std::bad_cast();
	}

	template<typename T>
	const T& get(const size_t index) const {
		if (type == NBTtagType::List) {
			return static_cast<T&>(data.List->at(index));
		}
		throw std::bad_cast();
	}

	template<typename T>
	const T& get(const std::string& key) const {
		if (type == NBTtagType::Compound) {
			return static_cast<T&>(data.Compound->at(key));
		}
		throw std::bad_cast();
	}
};


template<>
inline int8_t& NBT::at(const size_t index) {
	if (type == NBTtagType::ByteArray) {
		return (*data.ByteArray)[index];
	}
	throw std::bad_cast();
}

template<>
inline int32_t& NBT::at(const size_t index) {
	if (type == NBTtagType::IntArray) {
		return (*data.IntArray)[index];
	}
	throw std::bad_cast();
}

template<>
inline int64_t& NBT::at(const size_t index) {
	if (type == NBTtagType::LongArray) {
		return (*data.LongArray)[index];
	}
	throw std::bad_cast();
}


template<>
inline const int8_t& NBT::get(const size_t index) const {
	if (type == NBTtagType::ByteArray) {
		return (*data.ByteArray)[index];
	}
	throw std::bad_cast();
}

template<>
inline const int32_t& NBT::get(const size_t index) const {
	if (type == NBTtagType::IntArray) {
		return (*data.IntArray)[index];
	}
	throw std::bad_cast();
}

template<>
inline const int64_t& NBT::get(const size_t index) const {
	if (type == NBTtagType::LongArray) {
		return (*data.LongArray)[index];
	}
	throw std::bad_cast();
}
