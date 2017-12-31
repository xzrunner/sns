#include "sns/NodeSprBase.h"
#include "sns/ColorParser.h"

#include <bs/ImportStream.h>
#include <bs/ExportStream.h>
#include <bs/typedef.h>
#include <bs/Serializer.h>
#include <bs/FixedPointNum.h>
#include <memmgr/LinearAllocator.h>

namespace sns
{

NodeSprBase::NodeSprBase()
	: m_sym_path(nullptr)
	, m_name(nullptr)
	, m_type(0)
	, m_data(nullptr)
{
}

size_t NodeSprBase::GetBinSize() const
{
	size_t sz = 0;
	sz += bs::pack_size(m_sym_path); // sym_path
	sz += bs::pack_size(m_name);     // name
	sz += bs::pack_size(m_type);     // type
	sz += DataSize(m_type);          // data
	return sz;
}

void NodeSprBase::StoreToBin(bs::ExportStream& es) const
{
	es.Write(m_sym_path); // sym_path
	es.Write(m_name);     // name
	es.Write(m_type);     // type
	int zz = DataSize(m_type);
	es.WriteBlock(reinterpret_cast<uint8_t*>(m_data), DataSize(m_type)); // data
}

void NodeSprBase::StoreToJson(rapidjson::Value& val) const
{

}

void NodeSprBase::LoadFromBin(mm::LinearAllocator& alloc, bs::ImportStream& is)
{
	m_sym_path = is.String(alloc);
	m_name = is.String(alloc);
	m_type = is.UInt32();

	int zz = DataSize(m_type);
	m_data = static_cast<uint32_t*>(alloc.alloc<char>(DataSize(m_type)));
	int idx = 0;
	if (m_type & SCALE_MASK) {
		m_data[idx++] = is.UInt32();
		m_data[idx++] = is.UInt32();
	}
	if (m_type & SHEAR_MASK) {
		m_data[idx++] = is.UInt32();
		m_data[idx++] = is.UInt32();
	}
	if (m_type & OFFSET_MASK) {
		m_data[idx++] = is.UInt32();
		m_data[idx++] = is.UInt32();
	}
	if (m_type & POSITION_MASK) {
		m_data[idx++] = is.UInt32();
		m_data[idx++] = is.UInt32();
	}
	if (m_type & ANGLE_MASK) {
		m_data[idx++] = is.UInt32();
	}
	if (m_type & COL_MUL_MASK) {
		m_data[idx++] = is.UInt32();
	}
	if (m_type & COL_ADD_MASK) {
		m_data[idx++] = is.UInt32();
	}
	if (m_type & COL_R_MASK) {
		m_data[idx++] = is.UInt32();
	}
	if (m_type & COL_G_MASK) {
		m_data[idx++] = is.UInt32();
	}
	if (m_type & COL_B_MASK) {
		m_data[idx++] = is.UInt32();
	}
	if (m_type & BLEND_MASK) {
		m_data[idx++] = is.UInt32();
	}
	if (m_type & FAST_BLEND_MASK) {
		m_data[idx++] = is.UInt32();
	}
	if (m_type & FILTER_MASK) {
		m_data[idx++] = is.UInt32();
	}
	if (m_type & DOWNSMAPLE_MASK) {
		m_data[idx++] = is.UInt32();
	}
	if (m_type & CAMERA_MASK) {
		m_data[idx++] = is.UInt32();
	}
}

void NodeSprBase::LoadFromJson(mm::LinearAllocator& alloc, const rapidjson::Value& val)
{
	// load name
	m_name = nullptr;
	if (val.HasMember("name")) {
		m_name = CopyJsonStr(alloc, val["name"]);
	}

	// load filepath
	m_sym_path = nullptr;
	if (val.HasMember("filepath")) {
		m_sym_path = CopyJsonStr(alloc, val["filepath"]);
	}

	m_type = 0;

	std::vector<uint32_t> data;

	// load scale
	float scale[2] = { 1, 1 };
	if (val.HasMember("x scale") && val.HasMember("y scale")) {
		scale[0] = val["x scale"].GetFloat();
		scale[1] = val["y scale"].GetFloat();
	} else if (val.HasMember("scale")) {
		scale[0] = scale[1] = val["scale"].GetFloat();
	}
	if (scale[0] != 1 || scale[1] != 1) {
		m_type |= SCALE_MASK;
		data.push_back(bs::float2int(scale[0], HIGH_FIXED_TRANS_PRECISION));
		data.push_back(bs::float2int(scale[1], HIGH_FIXED_TRANS_PRECISION));
	}

	// load shear
	float shear[2] = { 0, 0 };
	if (val.HasMember("x shear") && val.HasMember("y shear")) {
		shear[0] = val["x shear"].GetFloat();
		shear[1] = val["y shear"].GetFloat();
	}
	if (shear[0] != 0 || shear[1] != 0) {
		m_type |= SHEAR_MASK;
		data.push_back(bs::float2int(shear[0], HIGH_FIXED_TRANS_PRECISION));
		data.push_back(bs::float2int(shear[1], HIGH_FIXED_TRANS_PRECISION));
	}

	// load offset
	float offset[2] = { 0, 0 };
	if (val.HasMember("x offset") && val.HasMember("y offset"))
	{
		offset[0] = val["x offset"].GetFloat();
		offset[1] = val["y offset"].GetFloat();
	}
	if (offset[0] != 0 || offset[1] != 0) {
		m_type |= OFFSET_MASK;
		data.push_back(bs::float2int(offset[0], LOW_FIXED_TRANS_PRECISION));
		data.push_back(bs::float2int(offset[1], LOW_FIXED_TRANS_PRECISION));
	}

	// load position
	float position[2] = { 0, 0 };
	if (val.HasMember("position") && val["position"].HasMember("x") && val["position"].HasMember("y")) {
		position[0] = val["position"]["x"].GetFloat();
		position[1] = val["position"]["y"].GetFloat();
	}
	if (position[0] != 0 || position[1] != 0) {
		m_type |= POSITION_MASK;
		data.push_back(bs::float2int(position[0], LOW_FIXED_TRANS_PRECISION));
		data.push_back(bs::float2int(position[1], LOW_FIXED_TRANS_PRECISION));
	}

	// load rotate
	float angle = 0;
	if (val.HasMember("angle")) {
		angle = val["angle"].GetFloat();
	}
	if (angle != 0) {
		m_type |= SCALE_MASK;
		data.push_back(bs::float2int(angle, HIGH_FIXED_TRANS_PRECISION));
	}

	// load��color mul
	uint32_t col_mul = 0xffffffff;
	if (val.HasMember("multi color")) {
		auto& col_val(val["multi color"]);
		col_mul = ColorParser::StringToRGBA(col_val.GetString(), col_val.GetStringLength(), BGRA);
	}
	if (col_mul != 0xffffffff) {
		m_type |= COL_MUL_MASK;
		data.push_back(col_mul);
	}

	// load color add
	uint32_t col_add = 0;
	if (val.HasMember("add color")) {
		auto& col_val(val["add color"]);
		col_add = ColorParser::StringToRGBA(col_val.GetString(), col_val.GetStringLength(), ABGR);
	}
	if (col_add != 0) {
		m_type |= COL_ADD_MASK;
		data.push_back(col_add);
	}

	// load color rmap
	uint32_t col_rmap = 0xff000000;
	if (val.HasMember("r trans")) {
		auto& col_val(val["r trans"]);
		col_rmap = ColorParser::StringToRGBA(col_val.GetString(), col_val.GetStringLength(), RGBA);
		col_rmap &= 0xffffff00;
	}
	if (col_rmap != 0xff000000) {
		m_type |= COL_R_MASK;
		data.push_back(col_rmap);
	}

	// load color gmap
	uint32_t col_gmap = 0x00ff0000;
	if (val.HasMember("g trans")) {
		auto& col_val(val["g trans"]);
		col_gmap = ColorParser::StringToRGBA(col_val.GetString(), col_val.GetStringLength(), RGBA);
		col_gmap &= 0xffffff00;
	}
	if (col_gmap != 0x00ff0000) {
		m_type |= COL_G_MASK;
		data.push_back(col_gmap);
	}

	// load color bmap
	uint32_t col_bmap = 0x0000ff00;
	if (val.HasMember("b trans")) {
		auto& col_val(val["b trans"]);
		col_bmap = ColorParser::StringToRGBA(col_val.GetString(), col_val.GetStringLength(), RGBA);
		col_bmap &= 0xffffff00;
	}
	if (col_bmap != 0x0000ff00) {
		m_type |= COL_B_MASK;
		data.push_back(col_bmap);
	}

	// store
	if (data.empty())
	{
		m_data = nullptr;
	}
	else
	{
		size_t sz = sizeof(uint32_t) * data.size();
		m_data = static_cast<uint32_t*>(alloc.alloc<char>(sz));
		memcpy(m_data, &data[0], sz);
	}
}

size_t NodeSprBase::DataSize(uint32_t type)
{
	size_t sz = 0;

	if (type & SCALE_MASK) {
		sz += sizeof(uint32_t) * 2;
	}
	if (type & SHEAR_MASK) {
		sz += sizeof(uint32_t) * 2;
	}
	if (type & OFFSET_MASK) {
		sz += sizeof(uint32_t) * 2;
	}
	if (type & POSITION_MASK) {
		sz += sizeof(uint32_t) * 2;
	}
	if (type & ANGLE_MASK) {
		sz += sizeof(uint32_t);
	}
	if (type & COL_MUL_MASK) {
		sz += sizeof(uint32_t);
	}
	if (type & COL_ADD_MASK) {
		sz += sizeof(uint32_t);
	}
	if (type & COL_R_MASK) {
		sz += sizeof(uint32_t);
	}
	if (type & COL_G_MASK) {
		sz += sizeof(uint32_t);
	}
	if (type & COL_B_MASK) {
		sz += sizeof(uint32_t);
	}
	if (type & BLEND_MASK) {
		sz += sizeof(uint32_t);
	}
	if (type & FAST_BLEND_MASK) {
		sz += sizeof(uint32_t);
	}
	if (type & FILTER_MASK) {
		sz += sizeof(uint32_t);
	}
	if (type & DOWNSMAPLE_MASK) {
		sz += sizeof(uint32_t);
	}
	if (type & CAMERA_MASK) {
		sz += sizeof(uint32_t);
	}

	return ALIGN_4BYTE(sz);
}

char* NodeSprBase::CopyJsonStr(mm::LinearAllocator& alloc, const rapidjson::Value& val)
{
	size_t len = val.GetStringLength();
	if (len == 0) {
		return nullptr;
	}

	char* ret = static_cast<char*>(alloc.alloc<char>(len + 1));

	strncpy(ret, val.GetString(), len);
	ret[len] = 0;

	return ret;
}

}