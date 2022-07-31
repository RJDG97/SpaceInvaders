/******************************************************************************
filename: Components.h
author: Renzo Garcia renzo.garcia@digipen.edu
Project: CS396 Final - Space Invaders
Description: This file contains code components required for the game
******************************************************************************/

//---------------------------------------------------------------------------------------
// COMPONENTS
//---------------------------------------------------------------------------------------

/////////////////////////////////////////////////
struct position
{
    constexpr static auto typedef_v = xecs::component::type::data{};
    xcore::vector2 m_Value;
};

struct velocity
{
    constexpr static auto typedef_v = xecs::component::type::data{};
    xcore::vector2 m_Value;
};

struct timer
{
    constexpr static auto typedef_v = xecs::component::type::data{};

    bool active;
    float m_Value;
    float m_Timer;
};

struct grid_cell
{
    constexpr static auto typedef_v = xecs::component::type::share
    {
        .m_bBuildFilter = true
    };

    std::int16_t m_X;
    std::int16_t m_Y;
};

// ***********************************************

struct sprite
{
    constexpr static auto typedef_v = xecs::component::type::data{};
    int size;
    size_t width, height;
    uint8_t* data;
};

struct animation
{
    constexpr static auto typedef_v = xecs::component::type::data{};
    bool animate;
    uint8_t* sprite1;
    uint8_t* sprite2;
    bool issprite1;
    timer Timer;
};

struct player
{
    constexpr static auto typedef_v = xecs::component::type::tag{};
};

struct manager
{
    constexpr static auto typedef_v = xecs::component::type::tag{};
};

struct shield
{
    constexpr static auto typedef_v = xecs::component::type::tag{};
};

struct enemy
{
    constexpr static auto typedef_v = xecs::component::type::data{};

    bool isdead;
    int score;
    xcore::vector2 m_InitPos;
};

struct projectile
{
    constexpr static auto typedef_v = xecs::component::type::data{};

    xecs::component::entity m_ShipOwner;
    bool m_isplayer;
};

using projectile_tuple = std::tuple<position, velocity, projectile, grid_cell>;