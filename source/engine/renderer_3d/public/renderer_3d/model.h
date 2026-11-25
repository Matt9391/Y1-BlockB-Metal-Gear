/// @file
/// @brief glTF models: their nodes, meshes and animations. Load one with
/// buz::load_model from asset_management/model_loading.h, or read one
/// onto the cpu with buz::import_model.

#pragma once

#include "core/math.h"
#include "core/mesh_data.h"
#include "renderer_3d/material.h"

#include <string_view>
#include <memory>
#include <string>
#include <vector>

namespace buz
{
	class mesh;

	//--------------------------------------------------------------
	/// @brief Where a node of a model sits relative to its parent node: its
	/// translation, rotation and scale. A pose (a frame of an animation, or
	/// get_rest_pose) is one node_transform for every node.
	struct node_transform
	{
		/// @brief Where the node sits relative to its parent.
		vec3 translation = { 0.0f, 0.0f, 0.0f };
		/// @brief How the node is turned; the default is no rotation.
		quat rotation = quat(1.0f, 0.0f, 0.0f, 0.0f);
		/// @brief How much the node is scaled along each axis.
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
		node_transform local_transform;
		/// @brief The node's mesh; nullptr for an empty node.
		std::shared_ptr<mesh> shape;
		/// @brief How shape looks. Its base_texture names the image; load it
		/// with buz::load_texture(engine, look.base_texture).
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
		std::vector<std::vector<node_transform>> frames;
	};

	//--------------------------------------------------------------
	/// @brief One node of a model on the cpu, as buz::import_model reads it:
	/// a model_node whose triangles are not on the graphics card yet.
	struct model_node_data
	{
		/// @brief The node's name in Blender.
		std::string name;
		/// @brief Index of the parent node, -1 for a root.
		int parent = -1;
		/// @brief Where the node sits relative to its parent, as placed in the file.
		node_transform local_transform;
		/// @brief The node's triangles, in its own space; empty for an empty node.
		mesh_data triangles;
		/// @brief How the triangles look.
		material look;
	};

	//--------------------------------------------------------------
	/// @brief Everything in a glTF file, on the cpu: what buz::import_model
	/// reads. Merge, split or change the triangles before you upload them
	/// (buz::create_mesh); buz::load_model uploads every node as it is.
	struct model_data
	{
		/// @brief Every node; parents always come before their children.
		std::vector<model_node_data> nodes;
		/// @brief Every animation.
		std::vector<keyframe_clip> clips;
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
		std::vector<node_transform> get_rest_pose() const;
	};
}
