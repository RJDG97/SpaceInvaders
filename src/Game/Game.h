#pragma once

#include "Components/Components.h"

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
    int             m_Highscore = 0;
    int             score = 0;
    int             enemies = 0;
    int             lives = 3;
    int             level = 1;

    xecs::archetype::instance* m_pProjectileArchetype{};
    xecs::archetype::instance* m_pEnemyArchetype{};
    xecs::archetype::instance* m_pPlayerArchetype{};
    xecs::archetype::instance* m_pShieldArchetype{};
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

#include "Archetypes/Archetypes.h"
#include "Systems/Systems.h"