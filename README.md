# CS396 - Space Invaders

# Engine used xECS by LIONant
Go to the [main branch](https://github.com/LIONant-depot/xECS/tree/master) for the introduction.

Space Invaders
* Controls
  * w - Shoot
  * a - Move left
  * d - Move right
* Features
  * Highscore - Updates whenever a game ends | auto starts new game
  * Level - Enemies get faster as the level increases
  * Pixels - Game uses pixel art
  * Player - Has 3 lives | lose a life when shot | gain a life when enemy wave is defeated
  * Shields - Disintegrate by pixels
  * Enemies - Animated | Squid - 40 points | Alien - 20 points | Jellyfish - 10 points

## To build
1. Go to the build directory 
2. Click the GetDependencies.bat batch file
3. Wait untill it says that xECS is done
4. Go to the example project and load it, you should be able to run it at this point

## Dependencies
- [xCore](https://gitlab.com/LIONant/xcore) (for the profiler tracy, and few types )
- [CLUT](https://github.com/markkilgard/glut) (for the openGL example)

