#pragma once

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

struct bullet
{
    constexpr static auto typedef_v = xecs::component::type::data{};

    xecs::component::entity m_ShipOwner;
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

using bullet_tuple = std::tuple<position, velocity, timer, bullet, grid_cell>;

// ***********************************************

struct sprite
{
    constexpr static auto typedef_v = xecs::component::type::data{};

    size_t width, height;
    uint8_t* data;
};

struct animation
{
    constexpr static auto typedef_v = xecs::component::type::data{};
    uint8_t* sprite1;
    uint8_t* sprite2;
    bool issprite1;
    timer Timer;
};

struct player
{
    constexpr static auto typedef_v = xecs::component::type::tag{};
};

struct enemy
{
    constexpr static auto typedef_v = xecs::component::type::data{};

    int score;
};

struct projectile
{
    constexpr static auto typedef_v = xecs::component::type::tag{};
};

using projectile_tuple = std::tuple<position, velocity, projectile, grid_cell>;