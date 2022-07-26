#pragma once

#include "../Game.h"
#include "../Sprites.h"

namespace Archetypes
{

	void GeneratePlayer()
	{
        s_Game.m_GameMgr->getOrCreateArchetype< position, velocity, timer, sprite, player, grid_cell>()
            .CreateEntities(1, [&](position& Position, velocity& Velocity, timer& Timer, sprite& Sprite, grid_cell& Cell) noexcept
                {
                    Position.m_Value = xcore::vector2{ static_cast<float>(s_Game.m_W / 2)
                    , static_cast<float>(s_Game.m_H / 1.1f) };

                    Timer.active = false;
                    Timer.m_Value = 0.0f;
                    Timer.m_Timer = 0.5f;

                    Cell = grid::ComputeGridCellFromWorldPosition(Position.m_Value);

                    Sprite.width = Sprites::playerspritedata.width;
                    Sprite.height = Sprites::playerspritedata.height;
                    Sprite.data = Sprites::playerspritedata.sprite;
                });
	}

    void GenerateEnemy()
    {
        xecs::archetype::instance* m_pEnemyArchetype{};

        m_pEnemyArchetype = &s_Game.m_GameMgr->getOrCreateArchetype< position, velocity, timer, sprite, enemy, grid_cell, animation>();
        for (int y = 0; y < 5; ++y)
        {
            for (int x = 0; x < 11; ++x)
            {
                m_pEnemyArchetype->CreateEntity([&](position& Position, velocity& Velocity, timer& Timer, sprite& Sprite, grid_cell& Cell, animation& Animation, enemy& Enemy) noexcept
                    {
                        Position.m_Value = xcore::vector2{ static_cast<float>(s_Game.m_W/ 16 * x + (s_Game.m_W/5.4))
                        , static_cast<float>(s_Game.m_H/12  * y + 150) };

                        Enemy.m_InitPos = Position.m_Value;
                        Velocity.m_Value.m_X = 0.5f;
                        Cell = grid::ComputeGridCellFromWorldPosition(Position.m_Value);
                        if (y == 0)
                        {
                            // Sprite
                            Sprite.width = Sprites::alienspriteAdata.width;
                            Sprite.height = Sprites::alienspriteAdata.height;
                            Sprite.data = Sprites::alienspriteAdata.sprite1;
                            // Animation
                            Animation.sprite1 = Sprite.data;
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
                            Animation.sprite1 = Sprite.data;
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
                            Animation.sprite1 = Sprite.data;
                            Animation.sprite2 = Sprites::alienspriteCdata.sprite2;
                            Enemy.score = 10;
                        }

                        Animation.issprite1 = true;
                        Animation.Timer.active = true;
                        Animation.Timer.m_Value = 0.0f;
                        Animation.Timer.m_Timer = 0.1f;
                    });
            }
        }
    }
}