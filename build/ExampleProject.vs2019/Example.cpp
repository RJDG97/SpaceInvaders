/******************************************************************************
filename: Example.cpp
author: Tomas Arce tomas.arcegil@digipen.edu,
        Renzo Garcia renzo.garcia@digipen.edu
Project: CS396 Final - Space Invaders
Description: This file contains the 'main' function. 
Program execution begins and ends there.
******************************************************************************/

#include "xecs.h"
#define GLUT_STATIC_LIB
#include "GL/glut.h"
#include <random>

#include "Game/Game.h"

void InitializeGame( void ) noexcept
{
    //
    // Initialize global elements
    //
    std::srand(101);

    //
    // Register Components (They should always be first)
    //
    s_Game.m_GameMgr->RegisterComponents
    <   sprite
    ,   animation
    ,   manager
    ,   player
    ,   enemy
    ,   shield
    ,   position
    ,   velocity
    ,   timer
    ,   projectile
    ,   grid_cell
    >();

    //
    // Register Systems
    //

    // Register updated systems (the update system should be before the delegate systems)
    s_Game.m_GameMgr->RegisterSystems
    <   update_timer               // Structural: No
    ,   update_input               // Structural: Yes, Create(Bullets)
    ,   update_movement            // Structural: No
    ,   update_enemy_logic         // Structural: Yes, Create(Bullets || Ships)
    ,   update_projectile_movement // Structural: Yes, Destroy(Bullets || Ships)
    ,   update_projectile_logic    // Structural: Yes, Destroy(Bullets || Ships)
    ,   update_manager             // Structural: Yes, Destroy/Create(Bullets || Ships)
    ,   animate_enemy              // Structural: No
    ,   renderer                   // Structural: No
    // ,       render_grid            // Structural: No
    ,       update_ui              // Structural: No
    ,       render_border          // Structural: No
    ,       render_player          // Structural: No
    ,       render_enemy           // Structural: No
    ,       render_shield          // Structural: No
    ,       render_projectile      // Structural: No
    >();

    s_Game.m_GameMgr->getOrCreateArchetype< manager>().CreateEntity();
}

//---------------------------------------------------------------------------------------
// GLUT TIMER
//---------------------------------------------------------------------------------------

void timer( int value ) noexcept
{
    // Post re-paint request to activate display()
    glutPostRedisplay();

    // next timer call milliseconds later
    glutTimerFunc(15, timer, 0);
}

//---------------------------------------------------------------------------------------
// MAIN
//---------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    xcore::Init("ECS Example");

    //
    // Initialize the game
    //
    InitializeGame();

    //
    // Create the graphics and main loop
    //
    glutInitWindowSize(s_Game.m_W, s_Game.m_H);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutCreateWindow(xcore::get().m_pAppName);
    glutDisplayFunc([]( void ) noexcept
    {
        s_Game.m_GameMgr->Run();
    });
    glutReshapeFunc([](int w, int h) noexcept
    {
        s_Game.m_W = w;
        s_Game.m_H = h;
    });
    glutTimerFunc( 0, timer, 0 );
    glutKeyboardFunc([](unsigned char Key, int MouseX, int MouseY) noexcept
    { 
        s_Game.m_Keys[Key] = true;
        s_Game.m_MouseX = MouseX;
        s_Game.m_MouseY = MouseY;
    });
    glutKeyboardUpFunc([](unsigned char Key, int MouseX, int MouseY) noexcept
    {
        s_Game.m_Keys[Key] = false;
        s_Game.m_MouseX = MouseX;
        s_Game.m_MouseY = MouseY;
    });
    glutMouseFunc([](int Button, int State, int MouseX, int MouseY) noexcept
    {
        s_Game.m_MouseX = MouseX;
        s_Game.m_MouseY = MouseY;
             if (Button == GLUT_LEFT_BUTTON ) s_Game.m_MouseLeft  = (State == GLUT_DOWN);
        else if (Button == GLUT_RIGHT_BUTTON) s_Game.m_MouseRight = (State == GLUT_DOWN);
    });
    glutMainLoop();

    xcore::Kill();
    return 0;
}

