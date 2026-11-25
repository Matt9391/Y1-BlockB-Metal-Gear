# The game classes a game derives from: game, game_2d, and game_3d for the
# 3D template. buz::game implements the engine's buz::playable, which is all
# the engine knows about them, so this module sits on top of the engine. A
# different way of structuring games (an entity component system, say) can
# replace this module by implementing buz::playable, without changes to the
# engine.
BUZ_MODULE(game_framework
           PUBLIC_DEPS buz_engine buz_renderer_2d buz_renderer_3d
           PRIVATE_DEPS buz_renderer_2d_internal)
