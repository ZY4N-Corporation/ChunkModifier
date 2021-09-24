#include <NBT.hpp>

#include <functional>
#include <string.h>

std::string NBT::typeToString(NBTtagType type) {
	switch (type) {
	case NBTtagType::Boolean:	return "Boolean";
	case NBTtagType::Byte:		return "Byte";
	case NBTtagType::Short:		return "Short";
	case NBTtagType::Int:		return "Int";
	case NBTtagType::Long:		return "Long";
	case NBTtagType::Float:		return "Float";
	case NBTtagType::Double:	return "Double";
	case NBTtagType::String:	return "String";
	case NBTtagType::ByteArray:	return "ByteArray";
	case NBTtagType::IntArray:	return "IntArray";
	case NBTtagType::LongArray:	return "LongArray";
	case NBTtagType::List:		return "List";
	case NBTtagType::Compound:	return "Compound";
	case NBTtagType::null:		return "null";
	case NBTtagType::end:		return "end";
	default: return std::string("invalid NBT type ") + std::to_string(static_cast<uint8_t>(type));
	}
}


//--------------/ constructors /--------------//

NBT::NBT() : type(NBTtagType::null) {
	data.String = nullptr;
}

NBT::NBT(bool b) : type(NBTtagType::Boolean) {
	data.Boolean = b;
}

NBT::NBT(int8_t b) : type(NBTtagType::Byte) {
	data.Byte = b;
}

NBT::NBT(int16_t s) : type(NBTtagType::Short) {
	data.Short = s;
}

NBT::NBT(int32_t i) : type(NBTtagType::Int) {
	data.Int = i;
}

NBT::NBT(int64_t i) : type(NBTtagType::Long) {
	data.Long = i;
}

NBT::NBT(float f) : type(NBTtagType::Float) {
	data.Float = f;
}

NBT::NBT(double d) : type(NBTtagType::Double) {
	data.Double = d;
}

NBT::NBT(const NBTstring& s) : type(NBTtagType::String) {
	data.String = new NBTstring(s);
}

NBT::NBT(const NBTbyteArray& v) : type(NBTtagType::ByteArray) {
	data.ByteArray = new NBTbyteArray(v);
}

NBT::NBT(const NBTintArray& v) : type(NBTtagType::IntArray) {
	data.IntArray = new NBTintArray(v);
}

NBT::NBT(const NBTlongArray& v) : type(NBTtagType::LongArray) {
	data.LongArray = new NBTlongArray(v);
}

NBT::NBT(const NBTlist& v) : type(NBTtagType::List) {
	data.List = new NBTlist(v);
}

NBT::NBT(const NBTcompound& c) : type(NBTtagType::Compound) {
	data.Compound = new NBTcompound(c);
}


NBT::NBT(NBTstring&& s) : type(NBTtagType::String) {
	data.String = new NBTstring(std::move(s));
}

NBT::NBT(NBTbyteArray&& v) : type(NBTtagType::ByteArray) {
	data.ByteArray = new NBTbyteArray(std::move(v));
}

NBT::NBT(NBTintArray&& v) : type(NBTtagType::IntArray) {
	data.IntArray = new NBTintArray(std::move(v));
}

NBT::NBT(NBTlongArray&& v) : type(NBTtagType::LongArray) {
	data.LongArray = new NBTlongArray(std::move(v));
}

NBT::NBT(NBTlist&& v) : type(NBTtagType::List) {
	data.List = new NBTlist(std::move(v));
}

NBT::NBT(NBTcompound&& m) : type(NBTtagType::Compound) {
	data.Compound = new NBTcompound(std::move(m));
}


NBT::NBT(const NBT& nbt) {
	this->type = nbt.type;
	this->data = nbt.data;

	switch (type) {
	case NBTtagType::String:
		data.String		= new NBTstring(*data.String);			break;
	case NBTtagType::ByteArray:
		data.ByteArray	= new NBTbyteArray(*data.ByteArray);	break;
	case NBTtagType::IntArray:
		data.IntArray	= new NBTintArray(*data.IntArray);		break;
	case NBTtagType::LongArray:
		data.LongArray	= new NBTlongArray(*data.LongArray);	break;
	case NBTtagType::List:
		data.List		= new NBTlist(*data.List);				break;
	case NBTtagType::Compound:
		data.Compound	= new NBTcompound(*data.Compound);		break;
	}
}

NBT::NBT(NBT&& nbt) noexcept {
	this->type = nbt.type;
	this->data = nbt.data;

	nbt.type = NBTtagType::null;
	nbt.data.String = nullptr;
}

NBT::~NBT() {
	cleanup();
}


//--------------/ parsing /--------------//

NBT NBT::parse(uint8_t* buffer, size_t size) {
	BEstream is(buffer, size);

	NBTtagType type = static_cast<NBTtagType>(is.buffer[is.index++]);

	if (type != NBTtagType::Compound && type != NBTtagType::List)
		throw std::runtime_error(std::string("Invalid NBT type ") + std::to_string(static_cast<uint8_t>(type)));

	uint16_t length;
	is >> length;
	is.index += length;

	if (type == NBTtagType::Compound) {
		return NBT::parseCompound(is);
	}
	else {
		return NBT::parseList(is);
	}
}

const auto NBT::getParser(NBTtagType type) {
	switch (type) {
	case NBTtagType::Byte:		return parseByte;
	case NBTtagType::Short:		return parseShort;
	case NBTtagType::Int:		return parseInt;
	case NBTtagType::Long:		return parseLong;
	case NBTtagType::String:	return parseString;
	case NBTtagType::ByteArray:	return parseByteArray;
	case NBTtagType::IntArray:	return parseIntArray;
	case NBTtagType::LongArray:	return parseLongArray;
	case NBTtagType::List:		return parseList;
	case NBTtagType::Compound:	return parseCompound;
	case NBTtagType::Boolean:	return parseBool;
	default: throw std::runtime_error(std::string("Cannot parse NBT type ") + typeToString(type));
	}
}

NBT NBT::parseByte(BEstream& is) {
	return static_cast<int8_t>(is.buffer[is.index++]);
}

NBT NBT::parseBool(BEstream& is) {
	return static_cast<bool>(is.buffer[is.index++]);
}

NBT NBT::parseShort(BEstream& is) {
	int16_t value;
	is >> value;
	return value;
}

NBT NBT::parseInt(BEstream& is) {
	int32_t value;
	is >> value;
	return value;
}

NBT NBT::parseLong(BEstream& is) {
	int64_t value;
	is >> value;
	return value;
}

NBT NBT::parseString(BEstream& is) {
	NBTstring value;
	is >> value;
	return std::move(value);
}

NBT NBT::parseByteArray(BEstream& is) {
	uint32_t length; is >> length;
	NBTbyteArray array(length);
	for (size_t i = 0; i < length; i++) {
		is >> array[i];
	}
	return std::move(array);
}

NBT NBT::parseIntArray(BEstream& is) {
	uint32_t length; is >> length;
	NBTintArray array(length);
	for (size_t i = 0; i < length; i++) {
		is >> array[i];
	}
	return std::move(array);
}

NBT NBT::parseLongArray(BEstream& is) {
	uint32_t length; is >> length;
	NBTlongArray array(length);
	for (size_t i = 0; i < length; i++) {
		is >> array[i];
	}
	return std::move(array);
}

NBT NBT::parseList(BEstream& is) {
	NBTtagType contentType = static_cast<NBTtagType>(is.buffer[is.index++]);
	uint32_t length; is >> length;
	NBTlist list(length);

	if (static_cast<int8_t>(contentType) > 0) {
		const auto& parser = getParser(contentType);
		for (uint32_t i = 0; i < length; i++) {
			list[i] = parser(is);
		}
	}

	return std::move(list);
}

NBT NBT::parseCompound(BEstream& is) {
	NBTcompound compound;
	NBTtagType type;
	while ((type = static_cast<NBTtagType>(is.buffer[is.index++])) != NBTtagType::end) {
		std::string name; is >> name;
		compound.insert({ name, getParser(type)(is) });
	}
	return std::move(compound);
}


//--------------/ size & length /--------------//

size_t NBT::size(bool firstCall) const {
	
	size_t size = firstCall ? (sizeof(NBTtagType) + sizeof(uint16_t)) : 0; //empty string bc notch
	
	switch (type) {
	case NBTtagType::Boolean:	size += sizeof(bool);		break;
	case NBTtagType::Byte:		size += sizeof(int8_t);		break;
	case NBTtagType::Short:		size += sizeof(int16_t);	break;
	case NBTtagType::Int:		size += sizeof(int32_t);	break;
	case NBTtagType::Long:		size += sizeof(int64_t);	break;
	case NBTtagType::Float:		size += sizeof(float);		break;
	case NBTtagType::Double:	size += sizeof(double);		break;
	case NBTtagType::String:	size += sizeof(int16_t) + data.String->length() * sizeof(char);		break;
	case NBTtagType::ByteArray:	size += sizeof(int32_t) + data.ByteArray->size() * sizeof(int8_t);	break;
	case NBTtagType::IntArray:	size += sizeof(int32_t) + data.IntArray->size() * sizeof(int32_t);	break;
	case NBTtagType::LongArray:	size += sizeof(int32_t) + data.LongArray->size() * sizeof(int64_t);	break;
	case NBTtagType::List:
		size += sizeof(uint32_t) + (sizeof(NBTtagType) + sizeof(uint16_t)) * data.List->size();
		for (const auto& element : *data.List) {
			size += element.size(false);
		}
		size += sizeof(NBTtagType::end);
		break;
	case NBTtagType::Compound:
		for (const auto& element : *data.Compound) {
			size += sizeof(NBTtagType) + sizeof(uint16_t) + element.first.length();
			size += element.second.size(false);
		}
		size += sizeof(NBTtagType::end);
	}
	return size;
}

size_t NBT::length() {
	switch (type) {
	case NBTtagType::ByteArray:	return data.ByteArray->size();
	case NBTtagType::IntArray:	return data.IntArray->size();
	case NBTtagType::LongArray:	return data.LongArray->size();
	case NBTtagType::List:		return data.List->size();
	case NBTtagType::Compound:	return data.Compound->size();
	default: throw std::bad_cast();
	}
}


//--------------/ serialization /--------------//

uint8_t* NBT::serialize(size_t& size) const {
	size = this->size();
	uint8_t* bytes = new uint8_t[size];
	BEstream os(bytes, size);

	uint16_t emptyName = 0;
	os << this->type << emptyName;

	this->serialize(os);
	return bytes;
}

void NBT::serialize(BEstream& os) const {
	switch (type) {
	case NBTtagType::Boolean:
		os.buffer[os.index++] = static_cast<uint8_t>(data.Boolean);	break;
	case NBTtagType::Byte:
		os.buffer[os.index++] = static_cast<uint8_t>(data.Byte);	break;
	case NBTtagType::Short:
		os << data.Short;	break;
	case NBTtagType::Int:
		os << data.Int;		break;
	case NBTtagType::Long:
		os << data.Long;	break;
	case NBTtagType::Float:
		os << data.Float;	break;
	case NBTtagType::Double:
		os << data.Double;	break;
	case NBTtagType::String:
		os << *data.String;	break;
	case NBTtagType::ByteArray:
		os << static_cast<int32_t>(data.ByteArray->size());
		for (const int8_t num : *data.ByteArray) os << num;
		break;
	case NBTtagType::IntArray:
		os << static_cast<int32_t>(data.IntArray->size());
		for (const int32_t num : *data.IntArray) os << num;
		break;
	case NBTtagType::LongArray:
		os << static_cast<int32_t>(data.LongArray->size());
		for (const int64_t num : *data.LongArray) os << num;
		break;
	case NBTtagType::List:
		os << (data.List->size() > 0 ? data.List->at(0).type : NBTtagType::end);
		os << static_cast<int32_t>(data.List->size());
		for (const auto& nbt : *data.List) {
			nbt.serialize(os);
		}
		break;
	case NBTtagType::Compound:
		for (const auto& element : *data.Compound) {
			os << element.second.type << element.first;
			element.second.serialize(os);
		}
		os << NBTtagType::end;
	}
}


//--------------/ string/JSON conversion /--------------//

template<typename T>
void arrayToString(std::ostream& out, const std::vector<T>* array, int indent = -1) {
	const std::string indentTabs = (indent < 0 ? "" : std::string(indent, '\t'));
	const std::string lineBreak = (indent < 0 ? "" : "\n");

	out << '[' << lineBreak;
	const auto last = std::prev(array->end());
	for(auto it = array->begin(); it != array->end(); it++) {
		out << indentTabs << std::to_string(*it);
		if (it == last) {
			out << lineBreak;
			break;
		}
		out << ',' << lineBreak;
	}
	out << (indent < 0 ? "" : std::string(--indent, '\t')) << ']';
}


void NBT::toString(std::stringstream& out, int indent) const {

	const std::string indentTabs = (indent < 0 ? "" : std::string(++indent, tab));
	const std::string lineBreak = (indent < 0 ? "" : "\n");

	switch (type) {
	case NBTtagType::Boolean:	out << (data.Boolean ? "true" : "false");  break;
	case NBTtagType::Byte:		out << std::to_string(data.Byte);	break;
	case NBTtagType::Short:		out << std::to_string(data.Short);	break;
	case NBTtagType::Int:		out << std::to_string(data.Int);	break;
	case NBTtagType::Long:		out << std::to_string(data.Long);	break;
	case NBTtagType::Float:		out << std::to_string(data.Float);	break;
	case NBTtagType::Double:	out << std::to_string(data.Double);	break;
	case NBTtagType::String:	out << '\"'  << *data.String << '\"';  break;
	case NBTtagType::ByteArray:	arrayToString(out, data.ByteArray, indent);	break;
	case NBTtagType::IntArray:	arrayToString(out, data.IntArray, indent);	break;
	case NBTtagType::LongArray:	arrayToString(out, data.LongArray, indent);	break;
	case NBTtagType::List: {
		out << '[' << lineBreak;
		const auto last = std::prev(data.List->end());
		for (auto it = data.List->begin(); it != data.List->end(); it++) {
			out << indentTabs;
			it->toString(out, indent);
			if (it == last) {
				out << lineBreak;
				break;
			}
			out << ',' << lineBreak;
		}
		out << (indent < 0 ? "" : std::string(--indent, tab)) << ']';
		break;
	}
	case NBTtagType::Compound: {
		out << '{' << lineBreak;
		const auto last = std::prev(data.Compound->end());
		for(auto it = data.Compound->begin(); it != data.Compound->end(); it++) {
			out << indentTabs << '\"' << it->first << "\": ";
			it->second.toString(out, indent);
			if (it == last) {
				out << lineBreak;
				break;
			}
			out << ',' << lineBreak;
		}
		out << (indent < 0 ? "" : std::string(--indent, tab)) << '}';
		break;
	}
	default: out << std::string("null");
	}
}


//--------------/ cast operators /--------------//

NBT::operator bool() const {
	if (type == NBTtagType::Boolean)
		return data.Boolean;
	else throw std::bad_cast();
}

NBT::operator int8_t() const {
	switch (type) {
	case  NBTtagType::Byte:		return data.Byte;
	case  NBTtagType::Short:	return static_cast<int8_t>(data.Short);
	case  NBTtagType::Int:		return static_cast<int8_t>(data.Int);
	case  NBTtagType::Long:		return static_cast<int8_t>(data.Long);
	case  NBTtagType::Float:	return static_cast<int8_t>(data.Float);
	case  NBTtagType::Double:	return static_cast<int8_t>(data.Double);
	default: throw std::bad_cast();
	}
}

NBT::operator int16_t() const {
	switch (type) {
	case  NBTtagType::Short:	return data.Short;
	case  NBTtagType::Byte:		return static_cast<int16_t>(data.Byte);
	case  NBTtagType::Int:		return static_cast<int16_t>(data.Int);
	case  NBTtagType::Long:		return static_cast<int16_t>(data.Long);
	case  NBTtagType::Float:	return static_cast<int16_t>(data.Float);
	case  NBTtagType::Double:	return static_cast<int16_t>(data.Double);
	default: throw std::bad_cast();
	}
}

NBT::operator int32_t() const {
	switch (type) {
	case  NBTtagType::Int:		return data.Int;
	case  NBTtagType::Long:		return static_cast<int32_t>(data.Long);
	case  NBTtagType::Short:	return static_cast<int32_t>(data.Short);
	case  NBTtagType::Byte:		return static_cast<int32_t>(data.Byte);
	case  NBTtagType::Double:	return static_cast<int32_t>(data.Double);
	case  NBTtagType::Float:	return static_cast<int32_t>(data.Float);
	default: throw std::bad_cast();
	}
}

NBT::operator int64_t() const {
	switch (type) {
	case  NBTtagType::Long:		return data.Long;
	case  NBTtagType::Int:		return static_cast<int64_t>(data.Int);
	case  NBTtagType::Short:	return static_cast<int64_t>(data.Short);
	case  NBTtagType::Byte:		return static_cast<int64_t>(data.Byte);
	case  NBTtagType::Double:	return static_cast<int64_t>(data.Double);
	case  NBTtagType::Float:	return static_cast<int64_t>(data.Float);
	default: throw std::bad_cast();
	}
}

NBT::operator float() const {
	switch (type) {
	case  NBTtagType::Float:	return data.Float;
	case  NBTtagType::Double:	return static_cast<float>(data.Double);
	case  NBTtagType::Int:		return static_cast<float>(data.Int);
	case  NBTtagType::Long:		return static_cast<float>(data.Long);
	case  NBTtagType::Short:	return static_cast<float>(data.Short);
	case  NBTtagType::Byte:		return static_cast<float>(data.Byte);
	default: throw std::bad_cast();
	}
}

NBT::operator double() const {
	switch (type) {
	case  NBTtagType::Double:	return data.Double;
	case  NBTtagType::Float:	return static_cast<double>(data.Float);
	case  NBTtagType::Long:		return static_cast<double>(data.Long);
	case  NBTtagType::Int:		return static_cast<double>(data.Int);
	case  NBTtagType::Short:	return static_cast<double>(data.Short);
	case  NBTtagType::Byte:		return static_cast<double>(data.Byte);
	default: throw std::bad_cast();
	}
}


NBT::operator const NBTstring&() const {
	if (type == NBTtagType::String)
		return *data.String;
	throw std::bad_cast();
}

NBT::operator const NBTbyteArray&() const {
	if (type == NBTtagType::ByteArray)
		return *data.ByteArray;
	throw std::bad_cast();
}

NBT::operator const NBTintArray&() const {
	if (type == NBTtagType::IntArray)
		return *data.IntArray;
	throw std::bad_cast();
}

NBT::operator const NBTlongArray&() const {
	if (type == NBTtagType::LongArray)
		return *data.LongArray;
	throw std::bad_cast();
}

NBT::operator const NBTlist&() const {
	if (type == NBTtagType::List)
		return *data.List;
	throw std::bad_cast();
}

NBT::operator const NBTcompound&() const {
	if (type == NBTtagType::Compound)
		return *data.Compound;
	throw std::bad_cast();
}


NBT::operator bool&() {
	if (type == NBTtagType::Boolean)
		return data.Boolean;
	throw std::bad_cast();
}

NBT::operator int8_t&() {
	if (type == NBTtagType::Byte)
		return data.Byte;
	throw std::bad_cast();
}

NBT::operator int16_t&() {
	if (type == NBTtagType::Short)
		return data.Short;
	throw std::bad_cast();
}

NBT::operator int32_t&() {
	if (type == NBTtagType::Int)
		return data.Int;
	throw std::bad_cast();
}

NBT::operator int64_t&() {
	if (type == NBTtagType::Long)
		return data.Long;
	throw std::bad_cast();
}

NBT::operator float&() {
	if (type == NBTtagType::Float)
		return data.Float;
	throw std::bad_cast();
}

NBT::operator double&() {
	if (type == NBTtagType::Double)
		return data.Double;
	throw std::bad_cast();
}

NBT::operator NBTstring&() {
	if (type == NBTtagType::String)
		return *data.String;
	throw std::bad_cast();
}

NBT::operator NBTbyteArray&() {
	if (type == NBTtagType::ByteArray)
		return *data.ByteArray;
	throw std::bad_cast();
}

NBT::operator NBTintArray&() {
	if (type == NBTtagType::IntArray)
		return *data.IntArray;
	throw std::bad_cast();
}

NBT::operator NBTlongArray&() {
	if (type == NBTtagType::LongArray)
		return *data.LongArray;
	throw std::bad_cast();
}

NBT::operator NBTlist&() {
	if (type == NBTtagType::List)
		return *data.List;
	throw std::bad_cast();
}

NBT::operator NBTcompound&() {
	if (type == NBTtagType::Compound)
		return *data.Compound;
	throw std::bad_cast();
}


//--------------/ assignment operators /--------------//

NBT& NBT::operator=(bool b) {
	if (type == NBTtagType::null) {
		type = NBTtagType::Boolean;
	} else if (type != NBTtagType::Boolean) {
		throw std::bad_cast();
	}
	data.Boolean = b;
	return *this;
}

NBT& NBT::operator=(int8_t i) {
	switch (type) {
	case NBTtagType::null:		type = NBTtagType::Byte;
	case NBTtagType::Byte:		data.Byte	= i; break;
	case NBTtagType::Short:		data.Short	= static_cast<int16_t>(i); break;
	case NBTtagType::Int:		data.Int	= static_cast<int32_t>(i); break;
	case NBTtagType::Long:		data.Long	= static_cast<int64_t>(i); break;
	case NBTtagType::Float:		data.Float	= static_cast<float>(i); break;
	case NBTtagType::Double:	data.Double	= static_cast<double>(i); break;
	default: throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(int16_t i) {
	switch (type) {
	case NBTtagType::null:		type = NBTtagType::Short;
	case NBTtagType::Short:		data.Short	= i; break;
	case NBTtagType::Byte:		data.Byte	= static_cast<int8_t>(i); break;
	case NBTtagType::Int:		data.Int	= static_cast<int32_t>(i); break;
	case NBTtagType::Long:		data.Long	= static_cast<int64_t>(i); break;
	case NBTtagType::Float:		data.Float	= static_cast<float>(i); break;
	case NBTtagType::Double:	data.Double	= static_cast<double>(i); break;
	default: throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(int32_t i) {
	switch (type) {
	case NBTtagType::null:		type = NBTtagType::Int;
	case NBTtagType::Int:		data.Int	= i; break;
	case NBTtagType::Long:		data.Long	= static_cast<int64_t>(i); break;
	case NBTtagType::Short:		data.Short	= static_cast<int16_t>(i); break;
	case NBTtagType::Byte:		data.Byte	= static_cast<int8_t>(i); break;
	case NBTtagType::Float:		data.Float	= static_cast<float>(i); break;
	case NBTtagType::Double:	data.Double	= static_cast<double>(i); break;
	default: throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(int64_t i) {
	switch (type) {
	case NBTtagType::null:		type = NBTtagType::Long;
	case NBTtagType::Long:		data.Long	= i; break;
	case NBTtagType::Int:		data.Int	= static_cast<int32_t>(i); break;
	case NBTtagType::Short:		data.Short	= static_cast<int16_t>(i); break;
	case NBTtagType::Byte:		data.Byte	= static_cast<int8_t>(i); break;
	case NBTtagType::Float:		data.Float	= static_cast<float>(i); break;
	case NBTtagType::Double:	data.Double	= static_cast<double>(i); break;
	default: throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(float f) {
	switch (type) {
	case NBTtagType::null:		type = NBTtagType::Float;
	case NBTtagType::Float:		data.Float	= f; break;
	case NBTtagType::Double:	data.Double	= static_cast<double>(f); break;
	case NBTtagType::Byte:		data.Byte	= static_cast<int8_t>(f); break;
	case NBTtagType::Short:		data.Short	= static_cast<int16_t>(f); break;
	case NBTtagType::Int:		data.Int	= static_cast<int32_t>(f); break;
	case NBTtagType::Long:		data.Long	= static_cast<int64_t>(f); break;
	default: throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(double d) {
	switch (type) {
	case NBTtagType::null:		type = NBTtagType::Double;
	case NBTtagType::Double:	data.Double = d; break;
	case NBTtagType::Float:		data.Float	= static_cast<float>(d); break;
	case NBTtagType::Byte:		data.Byte	= static_cast<int8_t>(d); break;
	case NBTtagType::Short:		data.Short	= static_cast<int16_t>(d); break;
	case NBTtagType::Int:		data.Int	= static_cast<int32_t>(d); break;
	case NBTtagType::Long:		data.Long	= static_cast<int64_t>(d); break;
	default: throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(const NBTstring& s) {
	if (type == NBTtagType::null) {
		type = NBTtagType::String;
		data.String = new NBTstring(s);
	} else if (type == NBTtagType::String) {
		*data.String = s;
	} else {
		throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(const NBTbyteArray& v) {
	if (type == NBTtagType::null) {
		type = NBTtagType::ByteArray;
		data.ByteArray = new NBTbyteArray(v);
	} else if (type == NBTtagType::ByteArray) {
		*data.ByteArray = v;
	} else {
		throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(const NBTintArray& v) {
	if (type == NBTtagType::null) {
		type = NBTtagType::IntArray;
		data.IntArray = new NBTintArray(v);
	} else if (type == NBTtagType::IntArray) {
		*data.IntArray = v;
	} else {
		throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(const NBTlongArray& v) {
	if (type == NBTtagType::null) {
		type = NBTtagType::LongArray;
		data.LongArray = new NBTlongArray(v);
	} else if (type == NBTtagType::LongArray) {
		*data.LongArray = v;
	} else {
		throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(const NBTlist& v) {
	if (type == NBTtagType::null) {
		type = NBTtagType::List;
		data.List = new NBTlist(v);
	} else if (type == NBTtagType::List) {
		*data.List = v;
	} else {
		throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(const NBTcompound& m) {
	if (type == NBTtagType::null) {
		type = NBTtagType::Compound;
		data.Compound = new NBTcompound(m);
	} else if (type == NBTtagType::Compound) {
		*data.Compound = m;
	} else {
		throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(NBTstring&& s) {
	if (type == NBTtagType::null) {
		type = NBTtagType::String;
		data.String = new NBTstring(std::move(s));
	} else if (type == NBTtagType::String) {
		*data.String = std::move(s);
	} else {
		throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(NBTbyteArray&& v) {
	if (type == NBTtagType::null) {
		type = NBTtagType::ByteArray;
		data.ByteArray = new NBTbyteArray(std::move(v));
	} else if (type == NBTtagType::ByteArray) {
		*data.ByteArray = std::move(v);
	} else {
		throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(NBTintArray&& v) {
	if (type == NBTtagType::null) {
		type = NBTtagType::IntArray;
		data.IntArray = new NBTintArray(std::move(v));
	} else if (type == NBTtagType::IntArray) {
		*data.IntArray = std::move(v);
	} else {
		throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(NBTlongArray&& v) {
	if (type == NBTtagType::null) {
		type = NBTtagType::LongArray;
		data.LongArray = new NBTlongArray(std::move(v));
	} else if (type == NBTtagType::LongArray) {
		*data.LongArray = std::move(v);
	} else {
		throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(NBTlist&& v) {
	if (type == NBTtagType::null) {
		type = NBTtagType::List;
		data.List = new NBTlist(std::move(v));
	} else if (type == NBTtagType::List) {
		*data.List = std::move(v);
	} else {
		throw std::bad_cast();
	}
	return *this;
}


NBT& NBT::operator=(NBTcompound&& m) {
	if (type == NBTtagType::null) {
		type = NBTtagType::Compound;
		data.Compound = new NBTcompound(std::move(m));
	} else if (type == NBTtagType::Compound) {
		*data.Compound = std::move(m);
	} else {
		throw std::bad_cast();
	}
	return *this;
}

NBT& NBT::operator=(const NBT& nbt) {

	if (type == NBTtagType::null) {
		switch (type) {
		case NBTtagType::String:	data.String		= new NBTstring(*nbt.data.String);			break;
		case NBTtagType::ByteArray:	data.ByteArray	= new NBTbyteArray(*nbt.data.ByteArray);	break;
		case NBTtagType::IntArray:	data.IntArray	= new NBTintArray(*nbt.data.IntArray);		break;
		case NBTtagType::LongArray:	data.LongArray	= new NBTlongArray(*nbt.data.LongArray);	break;
		case NBTtagType::List:		data.List		= new NBTlist(*nbt.data.List);				break;
		case NBTtagType::Compound:	data.Compound	= new NBTcompound(*nbt.data.Compound);		break;
		default: this->data = nbt.data;
		}
		this->type = nbt.type;
	} else if (type == nbt.type) {
		switch (type) {
		case NBTtagType::String:	*data.String	= *nbt.data.String;		break;
		case NBTtagType::ByteArray:	*data.ByteArray	= *nbt.data.ByteArray;	break;
		case NBTtagType::IntArray:	*data.IntArray	= *nbt.data.IntArray;	break;
		case NBTtagType::LongArray:	*data.LongArray	= *nbt.data.LongArray;	break;
		case NBTtagType::List:		*data.List		= *nbt.data.List;		break;
		case NBTtagType::Compound:	*data.Compound	= *nbt.data.Compound;	break;
		default: this->data = nbt.data;
		}
	}

	return *this;
}

NBT& NBT::operator=(NBT&& nbt) noexcept {
	if (this != &nbt) {
		cleanup();

		this->data = nbt.data;
		this->type = nbt.type;

		nbt.type = NBTtagType::null;
		nbt.data.String = nullptr;
	}
	return *this;
}


//--------------/ member access operators /--------------//

NBT& NBT::operator[](const size_t index) {
	if (type == NBTtagType::null) {
		data.List = new NBTlist();
		type = NBTtagType::List;
	}
	if (type == NBTtagType::List) {
		if (index == data.List->size()) {
			return *data.List->insert(data.List->begin() + index, NBT());
		} else {
			return data.List->at(index);
		}
	}
	throw std::bad_cast();
}

NBT& NBT::operator[](const std::string& s) {
	if (type == NBTtagType::null) {
		data.Compound = new NBTcompound();
		type = NBTtagType::Compound;
	}
	if (type == NBTtagType::Compound) {
		return (*data.Compound)[s];
	}
	else throw std::bad_cast();
}


void NBT::cleanup() {
	switch (type) {
	case NBTtagType::String:	delete data.String;		data.String = nullptr;	break;
	case NBTtagType::ByteArray:	delete data.ByteArray;	data.ByteArray = nullptr;	break;
	case NBTtagType::IntArray:	delete data.IntArray;	data.IntArray = nullptr;	break;
	case NBTtagType::LongArray:	delete data.LongArray;	data.LongArray = nullptr;	break;
	case NBTtagType::List:		delete data.List;		data.List = nullptr;	break;
	case NBTtagType::Compound:	delete data.Compound;	data.Compound = nullptr;	break;
	}
}
