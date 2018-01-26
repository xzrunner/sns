#pragma once

#include <rapidjson/document.h>

#include <string>

namespace mm { class LinearAllocator; }

namespace sns
{

class Utility
{
public:
	static char* CopyStr2Alloc(mm::LinearAllocator& alloc, const rapidjson::Value& val);
	static char* CopyStr2Alloc(mm::LinearAllocator& alloc, const std::string& str);

}; // Utility

}