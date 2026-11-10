/// @file
/// @brief buz's vector, matrix and color types, and the 2D projection the
/// engine needs.

#pragma once

// The engine uses glm (source/third_party/glm) for its math: a battle tested,
// header-only library that mirrors the vector math you write in shaders. This
// header gives the glm types buz names and adds the 2D projection the engine
// itself needs. Game code only includes "core/math.h" and works with the buz::
// names. The math of your game (the camera, transforms, interpolation) is
// yours to write.
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace buz
{
	//--------------------------------------------------------------
	// The glm types under their buz names. These are aliases, not wrappers:
	// buz::vec2 IS glm::vec2, so every glm function works on them directly.
	using vec2 = glm::vec2;
	using vec3 = glm::vec3;
	using vec4 = glm::vec4;
	using ivec2 = glm::ivec2;
	using uvec2 = glm::uvec2;
	using mat3 = glm::mat3;
	using mat4 = glm::mat4;

	// A rotation in 3D, stored as a quaternion. Quaternions interpolate
	// smoothly (slerp) and have no gimbal lock, which is why animations
	// store rotations this way.
	using quat = glm::quat;

	//--------------------------------------------------------------
	// The most used glm functions, made available as buz::... so game code
	// does not have to know where they come from.
	using glm::clamp;
	using glm::dot;
	using glm::length;
	using glm::distance;
	using glm::normalize;
	using glm::radians;
	using glm::degrees;
	using glm::cross;
	using glm::inverse;
	using glm::transpose;

	//--------------------------------------------------------------
	inline constexpr float k_pi = 3.14159265358979323846f;

	//--------------------------------------------------------------
	/// @brief RGBA color with components in the [0, 1] range.
	struct color
	{
		/// @brief Red.
		float r = 1.0f;
		/// @brief Green.
		float g = 1.0f;
		/// @brief Blue.
		float b = 1.0f;
		/// @brief Alpha: 1 is opaque, 0 is invisible.
		float a = 1.0f;

		/// @brief Compares all components; exact float equality, no epsilon.
		bool operator==(const color&) const = default;
	};

	//--------------------------------------------------------------
	/// @brief Builds an orthographic projection for 2D rendering.
	///
	/// Maps pixel coordinates to normalized device coordinates, with the
	/// origin (0, 0) in the top-left corner of the screen and the y axis
	/// pointing down (like most 2D drawing APIs).
	///
	/// Note on conventions: the engine follows glm, so matrices are stored
	/// column-major (m[column][row]) and positions are column vectors that
	/// are transformed as v' = M * v. The translation lives in the last
	/// column of the matrix.
	///
	/// @param width Width of the render area in pixels.
	/// @param height Height of the render area in pixels.
	/// @return Orthographic projection matrix.
	mat4 make_orthographic_projection(float width, float height);
}
