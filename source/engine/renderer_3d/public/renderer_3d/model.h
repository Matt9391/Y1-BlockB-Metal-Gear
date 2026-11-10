/// @file
/// @brief glTF models: their nodes, meshes and animations, and loading them.

#pragma once

#include "core/filesystem.h"
#include "core/math.h"
#include "renderer_3d/material.h"

#include <string_view>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace buz
{
	class engine;
	class mesh;

	//--------------------------------------------------------------
	/// @brief Where a part of a model sits relative to its parent: the
	/// "pose" of one joint.
	struct joint_pose
	{
		/// @brief Where the joint sits relative to its parent.
		vec3 translation = { 0.0f, 0.0f, 0.0f };
		/// @brief How the joint is turned; the default is no rotation.
		quat rotation = quat(1.0f, 0.0f, 0.0f, 0.0f);
		/// @brief How much the joint is scaled along each axis.
		vec3 scale = { 1.0f, 1.0f, 1.0f };
	};

	//--------------------------------------------------------------
	/// @brief One node of a model, as it was named and placed in Blender.
	///
	/// A character made of rigid parts (the PS1 way) is a hierarchy of nodes:
	/// pelvis -> torso -> head, upper arm -> lower arm, and so on, each part a
	/// mesh that moves with its node. A level is a flat list of nodes too:
	/// walls with meshes, and empty nodes whose names mark what they are
	/// (col_* collision boxes, patrol_* points, camera_* positions).
	struct model_node
	{
		/// @brief The node's name in Blender.
		std::string name;
		/// @brief Index of the parent node, -1 for a root.
		int parent = -1;
		/// @brief Where the node sits relative to its parent, as placed in the file.
		joint_pose local_pose;
		/// @brief The node's mesh; nullptr for an empty node.
		std::shared_ptr<mesh> shape;
		/// @brief How shape looks.
		material look;
	};

	//--------------------------------------------------------------
	/// @brief An animation, sampled into poses at a fixed frame rate: every
	/// frame has a pose for every node of the model.
	///
	/// Turning this into movement is up to you: keep the time, find the two
	/// frames around it, blend between them, loop.
	struct keyframe_clip
	{
		/// @brief The animation's name in Blender.
		std::string name;
		/// @brief How many frames were sampled per second.
		float frames_per_second = 30.0f;
		/// @brief A pose per node for every frame: frames[frame][node].
		std::vector<std::vector<joint_pose>> frames;
	};

	//--------------------------------------------------------------
	/// @brief Everything in a glTF file: the nodes and the animations.
	struct model
	{
		/// @brief Every node; parents always come before their children.
		std::vector<model_node> nodes;
		/// @brief Every animation.
		std::vector<keyframe_clip> clips;

		//----------------------------------------------------------
		/// @brief Returns the index of the first node with this name, or -1.
		int find_node(const std::string& name) const;

		//----------------------------------------------------------
		/// @brief Returns the clip with this name, or nullptr.
		const keyframe_clip* find_clip(const std::string& name) const;

		//----------------------------------------------------------
		/// @brief Returns every node's pose as placed in the file.
		std::vector<joint_pose> get_rest_pose() const;
	};

	//--------------------------------------------------------------
	/// @brief Loads a glTF model (.gltf or .glb) from the content folder.
	///
	/// Every call loads the file again and uploads new meshes: loading a
	/// model once and reusing it is up to you.
	/// @param engine Engine instance; its graphics card gets the meshes.
	/// @param content_relative_path Model file, relative to the content folder.
	/// @return The model, or nothing when it could not be loaded (the reason is logged).
	std::optional<model> load_model(engine& engine, std::string_view content_relative_path);

	//--------------------------------------------------------------
	/// @brief Loads every mesh of a glTF file as one mesh, placed as in the
	/// file: for props and level geometry that never move apart.
	/// @param engine Engine instance; its graphics card gets the mesh.
	/// @param content_relative_path Model file, relative to the content folder.
	/// @return The mesh, or nullptr when it could not be loaded.
	std::shared_ptr<mesh> load_mesh(engine& engine, std::string_view content_relative_path);
}
