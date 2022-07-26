#pragma once

#include "../Components/Components.h"
#include "../Game.h"

//---------------------------------------------------------------------------------------
// GAME
//---------------------------------------------------------------------------------------

static struct game
{
    using game_mgr_uptr = std::unique_ptr<xecs::game_mgr::instance>;
    using keys_array = std::array<bool, 0xff + 1>;

    game_mgr_uptr   m_GameMgr = std::make_unique<xecs::game_mgr::instance>();
    int             m_W = 1024;
    int             m_H = 800;
    int             m_MouseX{};
    int             m_MouseY{};
    bool            m_MouseLeft{};
    bool            m_MouseRight{};
    keys_array      m_Keys{};
    float           m_ProjectileSpeed = 5.0f;
    float           m_PlayerMaxSpeed = 5.0f;

} s_Game;

namespace grid
{
    constexpr int   cell_width_v = 64; // Keep this divisible by 2
    constexpr int   cell_height_v = 42; // Keep this divisible by 2
    constexpr int   max_resolution_width_v = 1024;
    constexpr int   max_resolution_height_v = 800;
    constexpr auto  cell_x_count = static_cast<std::int16_t>(max_resolution_width_v / cell_width_v + 1);
    constexpr auto  cell_y_count = static_cast<std::int16_t>(max_resolution_height_v / cell_height_v + 1);

    //---------------------------------------------------------------------------------------

    template<typename T_FUNCTION>
    constexpr __inline
        bool Search
        (xecs::system::instance& System
            , const xecs::component::share_filter& ShareFilter
            , const std::int16_t                    X
            , const std::int16_t                    Y
            , const xecs::query::instance& Query
            , T_FUNCTION&& Function
        ) noexcept
    {
        static constexpr auto Table = std::array< std::int16_t, 2 * 6 >
        { -1, 0
            , -1, 1
            , -1, 0
            , 0, 1
            , -1, 1
            , 0, 1
        };

        //
        // Search self first 
        //
        if (System.Foreach(ShareFilter, Query, std::forward<T_FUNCTION&&>(Function)))
            return true;

        //
        // Search neighbors 
        //
        int i = (Y & 1) * (2 * 3);
        for (std::int16_t y = std::max(0, Y - 1), end_y = std::min(cell_y_count - 1, Y + 1); y != end_y; ++y)
        {
            if (auto pShareFilter = System.findShareFilter(grid_cell{ .m_X = X + Table[i + 0], .m_Y = y });
                pShareFilter && System.Foreach
                (*pShareFilter
                    , Query
                    , std::forward<T_FUNCTION&&>(Function)
                )
                )
                return true;

            if (auto pShareFilter = System.findShareFilter(grid_cell{ .m_X = X + Table[i + 1], .m_Y = y });
                pShareFilter && System.Foreach
                (*pShareFilter
                    , Query
                    , std::forward<T_FUNCTION&&>(Function)
                )
                )
                return true;

            i += 2;
        }
        return false;
    }

    //---------------------------------------------------------------------------------------

    __inline constexpr
        grid_cell ComputeGridCellFromWorldPosition(const xcore::vector2 Position) noexcept
    {
        const auto X = static_cast<int>(Position.m_X / (cell_width_v / 2.0f));
        const auto Y = std::max(0, std::min(static_cast<int>(Position.m_Y / cell_height_v), cell_y_count - 1));
        const auto x = 1 & ((X ^ Y) & Y);
        return
        { static_cast<std::int16_t>(std::max(0, std::min(1 + ((X - x) >> 1), cell_x_count - 1)))
        , static_cast<std::int16_t>(Y)
        };
    }
}

//---------------------------------------------------------------------------------------
// SYSTEMS
//---------------------------------------------------------------------------------------

struct update_input : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::update
    {
        .m_pName = "update_input"
    };

    xecs::archetype::instance* m_pProjectileArchetype{};

    using query = std::tuple
        <
        xecs::query::must<player>
        >;

    void OnGameStart(void) noexcept
    {
        m_pProjectileArchetype = &getOrCreateArchetype<projectile_tuple>();
    }

    __inline
        void operator()(position& Position, velocity& Velocity, timer& Timer) const noexcept
    {
        if (s_Game.m_Keys['d'])
        {
            Velocity.m_Value.m_X += 1;
            if (Velocity.m_Value.m_X > s_Game.m_PlayerMaxSpeed)
                Velocity.m_Value.m_X = s_Game.m_PlayerMaxSpeed;
        }
        else if (s_Game.m_Keys['a'])
        {
            Velocity.m_Value.m_X -= 1;
            if (Velocity.m_Value.m_X < -1 * s_Game.m_PlayerMaxSpeed)
                Velocity.m_Value.m_X = -1 * s_Game.m_PlayerMaxSpeed;
        }
        else
            Velocity.m_Value.m_X = 0;

        // Shoot Projectiles
        if (s_Game.m_Keys['w'])
        {
            if (!Timer.active)
            {
                m_pProjectileArchetype->CreateEntity([&](position& Pos, velocity& Vel, grid_cell& GridCell) noexcept
                    {
                        Pos.m_Value = Position.m_Value;
                        Vel.m_Value.m_Y = -s_Game.m_ProjectileSpeed;
                        GridCell = grid::ComputeGridCellFromWorldPosition(Pos.m_Value);
                    });
                Timer.active = true;
            }
        }
    }
};

struct update_movement : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::update
    {
        .m_pName = "update_movement"
    };

    using query = std::tuple
        <
        xecs::query::none_of<projectile>
        >;

    __inline
        void operator()(position& Position, velocity& Velocity, timer& Timer, grid_cell& GridCell) const noexcept
    {
        Position.m_Value += Velocity.m_Value;

        // Bounce on edges
        if (Position.m_Value.m_X < 0)
        {
            Position.m_Value.m_X = 0;
            Velocity.m_Value.m_X = -Velocity.m_Value.m_X;
        }
        else if (Position.m_Value.m_X >= grid::max_resolution_width_v)
        {
            Position.m_Value.m_X = grid::max_resolution_width_v - 1;
            Velocity.m_Value.m_X = -Velocity.m_Value.m_X;
        }

        if (Position.m_Value.m_Y < 0)
        {
            Position.m_Value.m_Y = 0;
            Velocity.m_Value.m_Y = -Velocity.m_Value.m_Y;
        }
        else if (Position.m_Value.m_Y >= grid::max_resolution_height_v)
        {
            Position.m_Value.m_Y = grid::max_resolution_height_v - 1;
            Velocity.m_Value.m_Y = -Velocity.m_Value.m_Y;
        }

        // Update the grid cell base on our new position
        GridCell = grid::ComputeGridCellFromWorldPosition(Position.m_Value);
    }
};

struct update_enemy_logic : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::update
    {
        .m_pName = "update_enemy_logic"
    };

    using query = std::tuple
        <xecs::query::must<enemy>>;

    __inline
        void operator()(entity& Entity, position& Position, velocity& Velocity, grid_cell& Cell, enemy& Enemy) const noexcept
    {
        // Update Movement
        if (Enemy.m_InitPos.m_X + ((s_Game.m_W / 16) * 3) < Position.m_Value.m_X || Enemy.m_InitPos.m_X - ((s_Game.m_W / 16) * 3) > Position.m_Value.m_X)
        {
            Velocity.m_Value.m_X = -Velocity.m_Value.m_X;
            Position.m_Value.m_Y += s_Game.m_H / 48;
        }
    }
};

struct update_projectile_logic : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::update
    {
        .m_pName = "update_projectile_logic"
    };

    using query = std::tuple
        <xecs::query::must<projectile>>;

    __inline
        void operator()(entity& Entity, position& Position, velocity& Velocity, grid_cell& Cell) const noexcept
    {
        // Update Movement
        if (Position.m_Value.m_Y > 0 || Position.m_Value.m_Y < grid::max_resolution_height_v)
        {
            Position.m_Value += Velocity.m_Value;
            Cell = grid::ComputeGridCellFromWorldPosition(Position.m_Value);
        }
        else // Delete if off screen
            DeleteEntity(Entity);
    }
};

//---------------------------------------------------------------------------------------

struct update_timer : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::update
    {
        .m_pName = "update_timer"
    };

    __inline constexpr
        void operator()(entity& Entity, timer& Timer) const noexcept
    {
        if(Timer.active)
            Timer.m_Value += 0.01f;

        if (Timer.m_Value >= Timer.m_Timer)
        {
            Timer.m_Value = 0.0f;
            Timer.active = false;
        }
    }
};

//---------------------------------------------------------------------------------------

struct bullet_logic : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::update
    {
        .m_pName = "bullet_logic"
    };

    xecs::query::instance m_QueryBullets;
    xecs::query::instance m_QueryAny;

    using query = std::tuple
        <
        xecs::query::must<bullet>
        >;

    void OnGameStart(void) noexcept
    {
        m_QueryBullets.AddQueryFromTuple<query>();
        m_QueryAny.m_Must.AddFromComponents<position>();
    }

    __inline
        void OnUpdate(void) noexcept
    {
        //
        // Update all the bullets
        //
        for (std::int16_t Y = 0; Y < grid::cell_y_count; ++Y)
            for (std::int16_t X = 0; X < grid::cell_x_count; ++X)
            {
                auto pShareFilter = findShareFilter(grid_cell{ .m_X = X, .m_Y = Y });
                if (pShareFilter == nullptr) continue;

                Foreach(*pShareFilter, m_QueryBullets, [&](entity& Entity, const position& Position, const bullet& Bullet) constexpr noexcept
                    {
                        // If I am dead because some other bullet killed me then there is nothing for me to do...
                        if (Entity.isZombie()) return;

                        grid::Search(*this, *pShareFilter, X, Y, m_QueryAny, [&](entity& E, const position& Pos)  constexpr noexcept
                            {
                                if (E.isZombie()) return false;

                                // Our we checking against my self?
                                if (Entity == E) return false;

                                // Are we colliding with our own ship?
                                // If so lets just continue
                                if (Bullet.m_ShipOwner == E) return false;

                                if (constexpr auto distance_v = 3; (Pos.m_Value - Position.m_Value).getLengthSquared() < distance_v * distance_v)
                                {
                                    DeleteEntity(Entity);
                                    DeleteEntity(E);
                                    return true;
                                }

                                return false;
                            });
                    });
            }
    }
};

//---------------------------------------------------------------------------------------

struct space_ship_logic : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::update
    {
        .m_pName = "space_ship_logic"
    };

    xecs::archetype::instance* m_pBulletArchetype{};
    xecs::query::instance       m_QueryThinkingShipsOnly{};
    xecs::query::instance       m_QueryAnyShips{};

    void OnGameStart(void) noexcept
    {
        m_pBulletArchetype = &getOrCreateArchetype<bullet_tuple>();

        m_QueryThinkingShipsOnly.m_Must.AddFromComponents<position>();
        m_QueryThinkingShipsOnly.m_NoneOf.AddFromComponents<bullet, timer>();

        m_QueryAnyShips.m_Must.AddFromComponents<position>();
        m_QueryAnyShips.m_NoneOf.AddFromComponents<bullet>();
    }

    using query = std::tuple
        <
        xecs::query::must<xecs::component::share_as_data_exclusive_tag>
        >;

    __inline
        void operator()(const grid_cell& GridCell, const xecs::component::share_filter& ShareFilter) noexcept
    {
        Foreach(ShareFilter, m_QueryThinkingShipsOnly, [&](entity& Entity, const position& Position) constexpr noexcept
            {
                grid::Search(*this, ShareFilter, GridCell.m_X, GridCell.m_Y, m_QueryAnyShips, [&](const entity& E, const position& Pos) constexpr noexcept
                    {
                        // Don't shoot myself
                        if (Entity == E) return false;

                        auto        Direction = Pos.m_Value - Position.m_Value;
                        const auto  DistanceSquare = Direction.getLengthSquared();

                        // Shoot a bullet if close enough
                        if (constexpr auto min_distance_v = 60; DistanceSquare < min_distance_v * min_distance_v)
                        {
                            auto NewEntity = AddOrRemoveComponents<std::tuple<timer>>(Entity, [&](timer& Timer)
                                {
                                    Timer.m_Value = 8;
                                });

                            // After moving the entity all access to its components via the function existing parameters is consider a bug
                            // Since the entity has moved to a different archetype
                            assert(Entity.isZombie());

                            // Hopefully there is not system that intersects me and kills me
                            assert(!NewEntity.isZombie());

                            m_pBulletArchetype->CreateEntity([&](position& Pos, velocity& Vel, bullet& Bullet, timer& Timer, grid_cell& Cell) noexcept
                                {
                                    Direction /= std::sqrt(DistanceSquare);
                                    Vel.m_Value = Direction * 2.0f;
                                    Pos.m_Value = Position.m_Value + Vel.m_Value;

                                    Bullet.m_ShipOwner = NewEntity;

                                    Cell = grid::ComputeGridCellFromWorldPosition(Pos.m_Value);

                                    Timer.m_Value = 10;
                                });

                            return true;
                        }

                        return false;
                    });
            });
    }
};

//---------------------------------------------------------------------------------------

struct renderer : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::update
    {
        .m_pName = "renderer"
    };

    using update = xecs::event::instance<>;

    using events = std::tuple
        < update
        >;

    __inline
        void OnUpdate(void) noexcept
    {
        //
        // Begin of the rendering
        //
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        glViewport(0, 0, s_Game.m_W, s_Game.m_H);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, s_Game.m_W, 0, s_Game.m_H, -1, 1);
        glScalef(1, -1, 1);
        glTranslatef(0, -s_Game.m_H, 0);

        //
        // Let all the system that depends on me
        //
        SendEventFrom<update>(this);

        //
        // Page Flip
        //
//        glFlush();
        glutSwapBuffers();
    }
};

//---------------------------------------------------------------------------------------

template< typename... T_ARGS>
void GlutPrint(const int x, const int y, const char* const pFmt, T_ARGS&&... Args) noexcept
{
    std::array<char, 256> FinalString;
    const auto len = sprintf_s(FinalString.data(), FinalString.size(), pFmt, Args...);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, s_Game.m_W, 0, s_Game.m_H);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRasterPos2i(x, s_Game.m_H - (y + 20));
    for (int i = 0; i < len; ++i)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, FinalString[i]);
    }
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

//---------------------------------------------------------------------------------------

struct animate_enemy : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::update
    {
        .m_pName = "animate_enemy"
    };

    using query = std::tuple
        <
        xecs::query::must<entity, enemy>
        >;

    __inline
        void operator()(sprite& Sprite, animation& Animation) const noexcept
    {
        if (!Animation.Timer.active)
        {
            if (Animation.issprite1)
                Sprite.data = Animation.sprite2;
            else
                Sprite.data = Animation.sprite1;

            Animation.issprite1 = !Animation.issprite1;
            Animation.Timer.active = true;
        }
        else
            Animation.Timer.m_Value += 0.01f;

        if (Animation.Timer.m_Value >= Animation. Timer.m_Timer)
        {
            Animation.Timer.m_Value = 0.0f;
            Animation.Timer.active = false;
        }
    }
};

struct render_player : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::child_update<renderer, renderer::update>
    {
        .m_pName = "render_player"
    };

    using query = std::tuple
        <
            xecs::query::must<entity, player>
        >;

    void OnPreUpdate(void) noexcept
    {
        glBegin(GL_QUADS);
    }

    void OnPostUpdate(void) noexcept
    {
        glEnd();
    }

    __inline
        void operator()(const position& Position, const sprite& Sprite) const noexcept
    {
        constexpr auto Size = 3;

        glColor3f(0.3, 1.0, 0.5);

        for (int y = 0; y < Sprite.height; ++y)
        {
            for (int x = 0; x < Sprite.width; ++x)
            {
                if (Sprite.data[x + (Sprite.width * y)])
                {   // Position + enemy size
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Size) / 2) + (x * Size) - Size / 2,
                        (Position.m_Value.m_Y - (Sprite.height * Size) / 2) + (y * Size) - Size / 2);
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Size) / 2) + (x * Size) - Size / 2,
                        (Position.m_Value.m_Y - (Sprite.height * Size) / 2) + (y * Size) + Size / 2);
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Size) / 2) + (x * Size) + Size / 2,
                        (Position.m_Value.m_Y - (Sprite.height * Size) / 2) + (y * Size) + Size / 2);
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Size) / 2) + (x * Size) + Size / 2,
                        (Position.m_Value.m_Y - (Sprite.height * Size) / 2) + (y * Size) - Size / 2);
                }

            }
        }
    }
};

struct render_enemy : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::child_update<renderer, renderer::update>
    {
        .m_pName = "render_enemy"
    };

    using query = std::tuple
        <
        xecs::query::must<entity, enemy>
        >;

    void OnPreUpdate(void) noexcept
    {
        glBegin(GL_QUADS);
    }

    void OnPostUpdate(void) noexcept
    {
        glEnd();
    }

    __inline
        void operator()(const position& Position, const sprite& Sprite) const noexcept
    {
        constexpr auto Size = 3;

        glColor3f(1.0, 1.0, 1.0);

        for (int y = 0; y < Sprite.height; ++y)
        {
            for (int x = 0; x < Sprite.width; ++x)
            {
                if (Sprite.data[x + (Sprite.width * y)])
                {
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Size)/2) + (x * Size) - Size/2,
                        (Position.m_Value.m_Y - (Sprite.height * Size) / 2) + (y * Size) - Size/2);
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Size) / 2) + (x * Size) - Size/2,
                        (Position.m_Value.m_Y - (Sprite.height * Size) / 2) + (y * Size) + Size/2);
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Size) / 2) + (x * Size) + Size/2,
                        (Position.m_Value.m_Y - (Sprite.height * Size) / 2) + (y * Size) + Size/2);
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Size) / 2) + (x * Size) + Size/2,
                        (Position.m_Value.m_Y - (Sprite.height * Size) / 2) + (y * Size) - Size/2);
                }

            }
        }
    }
};

struct render_projectile : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::child_update<renderer, renderer::update>
    {
        .m_pName = "render_projectile"
    };

    using query = std::tuple
        <
        xecs::query::must<projectile>
        >;

    void OnPreUpdate(void) noexcept
    {
        glBegin(GL_QUADS);
    }

    void OnPostUpdate(void) noexcept
    {
        glEnd();
    }

    __inline
        void operator()(const position& Position) const noexcept
    {
        constexpr auto Size = 3;

        glColor3f(1.0, 1.0, 1.0);

        for (int y = 0; y < 3; ++y)
        {
            glVertex2i(Position.m_Value.m_X + (Size/2), Position.m_Value.m_Y - (Size/2) + (y * Size));
            glVertex2i(Position.m_Value.m_X + (Size/2), Position.m_Value.m_Y + (Size/2) + (y * Size));
            glVertex2i(Position.m_Value.m_X - (Size/2), Position.m_Value.m_Y + (Size/2) + (y * Size));
            glVertex2i(Position.m_Value.m_X - (Size/2), Position.m_Value.m_Y - (Size/2) + (y * Size));
        }
    }
};

//---------------------------------------------------------------------------------------

struct render_grid : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::child_update<renderer, renderer::update>
    {
        .m_pName = "render_grid"
    };

    using query = std::tuple
        <
        xecs::query::must<xecs::component::share_as_data_exclusive_tag>
        >;

    __inline
        void operator()(const grid_cell& GridCell, const xecs::component::share_filter& ShareFilter) noexcept
    {
        // Hide nodes where there are not entities
        if constexpr (false)
        {
            int nEntities = 0;
            for (auto& ArchetypeCell : ShareFilter.m_lEntries)
                for (auto& Family : ArchetypeCell.m_lFamilies)
                    nEntities += static_cast<int>(Family->m_DefaultPool.Size());
            if (nEntities == 0) return;
        }

        const float X = ((GridCell.m_X - 1) + 0.5f + (GridCell.m_Y & 1) * 0.5f) * grid::cell_width_v;
        const float Y = (GridCell.m_Y + 0.5f) * grid::cell_height_v;
        constexpr auto SizeX = grid::cell_width_v / 2.0f - 1;
        constexpr auto SizeY = grid::cell_height_v / 2.0f - 1;

        glBegin(GL_QUADS);
        glColor3f(0.25f, 0.25f, 0.25f);
        glVertex2i(X - SizeX, Y - SizeY);
        glVertex2i(X - SizeX, Y + SizeY);
        glVertex2i(X + SizeX, Y + SizeY);
        glVertex2i(X + SizeX, Y - SizeY);
        glEnd();

        enum print
        {
            NONE
            , FAMILIES
            , ARCHETYPES
            , ENTITIES
            , GRIDCELL_XY
        };

        // What are we printing?
        switch (print::NONE)
        {
        case print::ARCHETYPES:
        {
            glColor3f(1.0f, 1.0f, 1.0f);
            GlutPrint(X, Y - 15, "%d", ShareFilter.m_lEntries.size());
            break;
        }
        case print::FAMILIES:
        {
            int nFamilies = 0;
            for (auto& ArchetypeCell : ShareFilter.m_lEntries)
                nFamilies += static_cast<int>(ArchetypeCell.m_lFamilies.size());

            glColor3f(1.0f, 1.0f, 1.0f);
            GlutPrint(X, Y - 15, "%d", nFamilies);
            break;
        }
        case print::ENTITIES:
        {
            int nEntities = 0;
            for (auto& ArchetypeCell : ShareFilter.m_lEntries)
                for (auto& Family : ArchetypeCell.m_lFamilies)
                    nEntities += static_cast<int>(Family->m_DefaultPool.Size());

            glColor3f(1.0f, 1.0f, 1.0f);
            GlutPrint(X, Y - 15, "%d", nEntities);
            break;
        }
        case print::GRIDCELL_XY:
        {
            glColor3f(1.0f, 1.0f, 1.0f);
            GlutPrint(X - 23, Y - 15, "%d,%d", GridCell.m_X, GridCell.m_Y);
            break;
        }
        }

        //
        // Print how many archetypes we have so far
        //
        if constexpr (false)
        {
            glColor3f(1.0f, 1.0f, 1.0f);
            GlutPrint(0, 0, "#Archetypes: %d", s_Game.m_GameMgr->m_ArchetypeMgr.m_lArchetype.size());
        }
    }
};