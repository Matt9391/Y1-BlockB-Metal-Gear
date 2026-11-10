# A tiny "catch the ball" game, meant as a first project. To start a game of
# your own, copy this folder, rename it, and change sample_2d below into the
# new folder's name. Everything else (sources, shaders, content) is set up by
# BUZ_GAME, see cmake/buz_game.cmake.
BUZ_GAME(sample_2d TIER tmpl_2d
         LINKS buz_engine)
