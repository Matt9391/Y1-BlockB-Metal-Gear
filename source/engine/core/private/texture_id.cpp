#include "core/texture_id.h"

#include "core/paths.h"
#include "core/texture_source.h"

#include <fstream>
#include <iterator>
#include <vector>

namespace
{
	//--------------------------------------------------------------
	/// @brief The hash ids are compared by; 0 for no image, which no
	/// image's bytes hash to.
	uint64_t get_hash(const buz::texture_source* source)
	{
		return source != nullptr ? source->hash : 0;
	}
}

namespace buz
{
	//--------------------------------------------------------------
	uint64_t hash_image_bytes(std::span<const uint8_t> bytes)
	{
		uint64_t hash = 14695981039346656037ull;
		for (const uint8_t byte : bytes)
		{
			hash = (hash ^ byte) * 1099511628211ull;
		}
		return hash != 0 ? hash : 1;
	}

	//--------------------------------------------------------------
	texture_id::texture_id() = default;

	//--------------------------------------------------------------
	texture_id::texture_id(std::string_view content_relative_path)
	{
		auto source = std::make_shared<texture_source>();
		source->what = std::string(content_relative_path);
		source->path = source->what;

		std::vector<uint8_t> bytes;
		std::ifstream file(paths::get_content_path(content_relative_path), std::ios::binary);
		if (file)
		{
			bytes.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		}
		if (bytes.empty())
		{
			bytes.assign(content_relative_path.begin(), content_relative_path.end());
		}
		source->hash = hash_image_bytes(bytes);
		m_source = std::move(source);
	}

	//--------------------------------------------------------------
	texture_id::texture_id(std::shared_ptr<const texture_source> source)
		: m_source(std::move(source))
	{
	}

	//--------------------------------------------------------------
	bool texture_id::has_image() const
	{
		return m_source != nullptr;
	}

	//--------------------------------------------------------------
	const std::string& texture_id::what() const
	{
		static const std::string no_texture = "no texture";
		return m_source != nullptr ? m_source->what : no_texture;
	}

	//--------------------------------------------------------------
	const texture_source* texture_id::get_source() const
	{
		return m_source.get();
	}

	//--------------------------------------------------------------
	bool texture_id::operator==(const texture_id& other) const
	{
		return get_hash(m_source.get()) == get_hash(other.m_source.get());
	}

	//--------------------------------------------------------------
	std::strong_ordering texture_id::operator<=>(const texture_id& other) const
	{
		return get_hash(m_source.get()) <=> get_hash(other.m_source.get());
	}
}

//--------------------------------------------------------------
size_t std::hash<buz::texture_id>::operator()(const buz::texture_id& id) const noexcept
{
	return static_cast<size_t>(get_hash(id.get_source()));
}
