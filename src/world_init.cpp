#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>

// TODO: create entity functions
//Player creation function
Entity createPlayer(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::DEBUG_LINE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = vec2(0.0f, 0.0f);
	motion.position = position;
	motion.angle = 0.f;
	motion.scale = vec2(50.0f, 50.0f);
	registry.colors.emplace(entity, vec3(0.5f, 0.5f, 0.5f)); // Gray color
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
		GEOMETRY_BUFFER_ID::DEBUG_LINE });

	return entity;
}