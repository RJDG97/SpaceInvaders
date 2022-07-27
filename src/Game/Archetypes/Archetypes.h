#pragma once

#include "../Sprites.h"

namespace Archetypes
{
    void InitArchetypes()
    {
        s_Game.m_pProjectileArchetype = &s_Game.m_GameMgr->getOrCreateArchetype<projectile_tuple>();
        s_Game.m_pEnemyArchetype = &s_Game.m_GameMgr->getOrCreateArchetype< position, velocity, timer, sprite, enemy, grid_cell, animation>();
        s_Game.m_pPlayerArchetype = &s_Game.m_GameMgr->getOrCreateArchetype< position, velocity, timer, sprite, player, grid_cell>();
        s_Game.m_pShieldArchetype = &s_Game.m_GameMgr->getOrCreateArchetype< position, sprite, grid_cell, shield>();
        s_Game.m_GameMgr->getOrCreateArchetype< manager>().CreateEntity();
    }

	void GeneratePlayer()
	{
        s_Game.m_pPlayerArchetype->CreateEntity([&](position& Position, velocity& Velocity, timer& Timer, sprite& Sprite, grid_cell& Cell) noexcept
            {
                Position.m_Value = xcore::vector2{ static_cast<float>(s_Game.m_W / 2)
                , static_cast<float>(s_Game.m_H / 1.1f) };

                Timer.active = false;
                Timer.m_Value = 0.0f;
                Timer.m_Timer = 0.25f;

                Cell = grid::ComputeGridCellFromWorldPosition(Position.m_Value);

                Sprite.size = 3;
                Sprite.width = Sprites::playerspritedata.width;
                Sprite.height = Sprites::playerspritedata.height;
                Sprite.data = Sprites::playerspritedata.sprite;
            });
	}

    __inline
    void GenerateEnemy()
    {
        for (int y = 0; y < 5; ++y)
        {
            for (int x = 0; x < 11; ++x)
            {
                s_Game.m_pEnemyArchetype->CreateEntity([&](position& Position, velocity& Velocity, timer& Timer, sprite& Sprite, grid_cell& Cell, animation& Animation, enemy& Enemy) noexcept
                    {
                        Position.m_Value = xcore::vector2{ static_cast<float>(s_Game.m_W/ 16 * x + (s_Game.m_W / 16 * 3))
                        , static_cast<float>(s_Game.m_H/12  * y + 150) };

                        Enemy.isdead = false;
                        Enemy.m_InitPos = Position.m_Value;
                        Velocity.m_Value.m_X = 0.5f + (s_Game.level * 0.2f);
                        Cell = grid::ComputeGridCellFromWorldPosition(Position.m_Value);
                        Sprite.size = 3;
                        if (y == 0)
                        {
                            // Sprite
                            Sprite.width = Sprites::alienspriteAdata.width;
                            Sprite.height = Sprites::alienspriteAdata.height;
                            Sprite.data = Sprites::alienspriteAdata.sprite1;
                            // Animation
                            Animation.sprite2 = Sprites::alienspriteAdata.sprite2;
                            Enemy.score = 40;
                        }
                        else if (y == 1 || y == 2)
                        {
                            // Sprite
                            Sprite.width = Sprites::alienspriteBdata.width;
                            Sprite.height = Sprites::alienspriteBdata.height;
                            Sprite.data = Sprites::alienspriteBdata.sprite1;
                            // Animation
                            Animation.sprite2 = Sprites::alienspriteBdata.sprite2;
                            Enemy.score = 20;
                        }
                        else if (y == 3 || y == 4)
                        {
                            // Sprite
                            Sprite.width = Sprites::alienspriteCdata.width;
                            Sprite.height = Sprites::alienspriteCdata.height;
                            Sprite.data = Sprites::alienspriteCdata.sprite1;
                            // Animation
                            Animation.sprite2 = Sprites::alienspriteCdata.sprite2;
                            Enemy.score = 10;
                        }
                        Animation.animate = true;
                        Animation.sprite1 = Sprite.data;
                        Animation.issprite1 = true;
                        Animation.Timer.active = true;
                        Animation.Timer.m_Value = 0.0f;
                        Animation.Timer.m_Timer = 0.1f;

                        Timer.active = true;
                        Timer.m_Value = 0.0f;
                        Timer.m_Timer = std::rand() / static_cast<float>(RAND_MAX) * 200;
                    });
                s_Game.enemies++;
            }
        }
    }

    void GenerateShields()
    {
        for (int x = 0; x < 6; ++x)
        {
            s_Game.m_pShieldArchetype->CreateEntity([&](position& Position, sprite& Sprite, grid_cell& Cell) noexcept
                {
                    Position.m_Value = xcore::vector2{ static_cast<float>(s_Game.m_W / 6 * (x) + (s_Game.m_W / 12))
                    , static_cast<float>(s_Game.m_H / 12 * 10) };

                    Cell = grid::ComputeGridCellFromWorldPosition(Position.m_Value);

                    // Sprite
                    Sprite.size = 5;
                    Sprite.width = Sprites::shieldspritedata.width;
                    Sprite.height = Sprites::shieldspritedata.height;
                    Sprite.data = new uint8_t[13 * 7]
                    {
                        0,0,0,1,1,1,1,1,1,1,0,0,0,
                        0,0,1,1,1,1,1,1,1,1,1,0,0,
                        0,1,1,1,1,1,1,1,1,1,1,1,0,
                        1,1,1,1,1,1,1,1,1,1,1,1,1,
                        1,1,1,1,1,1,1,1,1,1,1,1,1,
                        1,1,1,1,0,0,0,0,0,1,1,1,1,
                        1,1,1,0,0,0,0,0,0,0,1,1,1
                    };
                });
        }
    }
}