#pragma once
//---------------------------------------------------------------------------------------
// SYSTEMS
//---------------------------------------------------------------------------------------

struct update_input : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::update
    {
        .m_pName = "update_input"
    };

    using query = std::tuple
        <
        xecs::query::must<player>
        >;

    __inline
        void operator()(entity& Entity, position& Position, velocity& Velocity, timer& Timer) const noexcept
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
                s_Game.m_pProjectileArchetype->CreateEntity([&](position& Pos, velocity& Vel, grid_cell& GridCell, projectile& Projectile) noexcept
                    {
                        Pos.m_Value = Position.m_Value;
                        Vel.m_Value.m_Y = -s_Game.m_ProjectileSpeed;
                        GridCell = grid::ComputeGridCellFromWorldPosition(Pos.m_Value);
                        Projectile.m_ShipOwner = Entity;
                        Projectile.m_isplayer = true;
                    });
                Timer.active = true;
            }
        }
    }
};

//---------------------------------------------------------------------------------------

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

//---------------------------------------------------------------------------------------

struct update_enemy_logic : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::update
    {
        .m_pName = "update_enemy_logic"
    };

    using query = std::tuple
        <xecs::query::must<enemy>>;

    __inline
        void operator()(entity& Entity, position& Position, velocity& Velocity, timer& Timer, grid_cell& Cell, enemy& Enemy) const noexcept
    {
        if (Enemy.isdead)
        {
            if(!Timer.active)
                DeleteEntity(Entity);
            return;
        }

        // Update Movement
        if (Enemy.m_InitPos.m_X + ((s_Game.m_W / 17) * 3) < Position.m_Value.m_X || Enemy.m_InitPos.m_X - ((s_Game.m_W / 17) * 3) > Position.m_Value.m_X)
        {
            if (Velocity.m_Value.m_X < 0.0f)
                Velocity.m_Value.m_X -= 0.2f;
            else
                Velocity.m_Value.m_X += 0.2f;
            Velocity.m_Value.m_X = -Velocity.m_Value.m_X;
            Position.m_Value.m_Y += s_Game.m_H / 48;
        }
        if (!Timer.active)
        {
            s_Game.m_pProjectileArchetype->CreateEntity([&](position& Pos, velocity& Vel, grid_cell& GridCell, projectile& Projectile) noexcept
                {
                    Pos.m_Value = Position.m_Value;
                    Vel.m_Value.m_Y = +s_Game.m_ProjectileSpeed;
                    GridCell = grid::ComputeGridCellFromWorldPosition(Pos.m_Value);
                    Projectile.m_ShipOwner = Entity;
                    Projectile.m_isplayer = false;
                });
            Timer.active = true;
        }

        if (Position.m_Value.m_Y > s_Game.m_H / 5 * 4 - s_Game.m_H / 48)
            s_Game.lives = -1;
    }
};

//---------------------------------------------------------------------------------------

struct update_projectile_movement : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::update
    {
        .m_pName = "update_projectile_movement"
    };

    xecs::query::instance m_QueryEnemies;
    xecs::query::instance m_QueryPlayer;

    using query = std::tuple
        <xecs::query::must<projectile>>;

    void OnGameStart(void) noexcept
    {
        m_QueryPlayer.m_Must.AddFromComponents<player>();
        m_QueryEnemies.m_Must.AddFromComponents<enemy>();
    }

    __inline
        void operator()(entity& Entity, position& Position, velocity& Velocity, grid_cell& Cell, projectile& Projectile) const noexcept
    {
        // Update Movement
        if (Position.m_Value.m_Y > s_Game.m_H/12 * 2 && Position.m_Value.m_Y < s_Game.m_H / 12 * 11.4f)
        {
            Position.m_Value += Velocity.m_Value;
            Cell = grid::ComputeGridCellFromWorldPosition(Position.m_Value);
        }
        else // Delete if off screen
            DeleteEntity(Entity);
    }
};

//---------------------------------------------------------------------------------------

struct update_manager : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::update
    {
        .m_pName = "update_manager"
    };

    using query = std::tuple < xecs::query::must<manager> >;

    xecs::query::instance m_QueryPlayer;
    xecs::query::instance m_QueryEnemy;
    xecs::query::instance m_QueryProjectile;
    xecs::query::instance m_QueryShield;

    void OnGameStart(void) noexcept
    {
        m_QueryPlayer.m_Must.AddFromComponents<player>();
        m_QueryEnemy.m_Must.AddFromComponents<enemy>();
        m_QueryProjectile.m_Must.AddFromComponents<projectile>();
        m_QueryShield.m_Must.AddFromComponents<shield>();
    }

    __inline
        void operator()(entity& Entity) const noexcept
    {
        if (s_Game.enemies == 0)
        {
            if (s_Game.lives < 6)
                s_Game.lives++;
            Archetypes::GenerateEnemy();
        }

        if (s_Game.lives < 0)
        {
            if (s_Game.score > s_Game.m_Highscore)
            {
                s_Game.m_Highscore = s_Game.score;
            }
            s_Game.score = 0;
            s_Game.enemies = 0;
            s_Game.lives = 3;
            // Delete Player
            Foreach(Search(m_QueryPlayer), [&](entity& Entity) noexcept
                {
                    DeleteEntity(Entity);
                });
            Archetypes::GeneratePlayer();
            // Delete enemies
            Foreach(Search(m_QueryEnemy), [&](entity& Entity) noexcept
                {
                    DeleteEntity(Entity);
                });
            // Delete Bullets
            Foreach(Search(m_QueryProjectile), [&](entity& Entity) noexcept
                {
                    DeleteEntity(Entity);
                });

            Archetypes::GenerateEnemy();
            Foreach(Search(m_QueryShield), [&](entity& Entity) noexcept
                {
                    DeleteEntity(Entity);
                });
            Archetypes::GenerateShields();
        }
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

struct update_projectile_logic : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::update
    {
        .m_pName = "update_projectile_logic"
    };

    xecs::query::instance m_QueryProjectiles;
    xecs::query::instance m_QueryEnemy;
    xecs::query::instance m_QueryPlayer;
    xecs::query::instance m_QueryShield;

    using query = std::tuple
        <
        xecs::query::must<projectile>
        >;

    void OnGameStart(void) noexcept
    {
        m_QueryProjectiles.AddQueryFromTuple<query>();
        m_QueryPlayer.m_Must.AddFromComponents<position, sprite, player>();
        m_QueryEnemy.m_Must.AddFromComponents<position, sprite, enemy>();
        m_QueryShield.m_Must.AddFromComponents<position, sprite, shield>();
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

                Foreach(*pShareFilter, m_QueryProjectiles, [&](entity& Entity, const position& Position, const projectile& Projectile) constexpr noexcept
                    {
                        // If I am dead because some other bullet killed me then there is nothing for me to do...
                        if (Entity.isZombie()) return;

                        // Shield - projectile collision
                        grid::Search(*this, *pShareFilter, X, Y, m_QueryShield, [&](entity& E, const position& Pos, sprite& Sprite)  constexpr noexcept
                            {
                                if (E.isZombie()) return false;
                                if (Entity == E) return false;
                                if (Projectile.m_ShipOwner == E) return false;

                                for (int y = 0; y < Sprite.height; ++y)
                                {
                                    for (int x = 0; x < Sprite.width; ++x)
                                    {
                                        if (Sprite.data[x + (Sprite.width * y)])
                                        {
                                            if (Position.m_Value.m_X > (Pos.m_Value.m_X - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) - Sprite.size / 2
                                                && Position.m_Value.m_X < (Pos.m_Value.m_X - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) + Sprite.size / 2)
                                            {
                                                if (Position.m_Value.m_Y < static_cast<float>((Pos.m_Value.m_Y - (Sprite.height * Sprite.size) / 2.0f) + (y * Sprite.size) + Sprite.size/2)
                                                    && Position.m_Value.m_Y + 9 > static_cast<float>((Pos.m_Value.m_Y - (Sprite.height * Sprite.size) / 2.0f) + (y * Sprite.size) - Sprite.size/2))
                                                {
                                                    Sprite.data[x + (Sprite.width * y)] = 0;
                                                    Sprite.data[x + (Sprite.width * (y+1))] = 0;
                                                    DeleteEntity(Entity);
                                                    return true;
                                                }
                                            }
                                        }

                                    }
                                }
                                return false;
                            });

                        // Enemy - projectile collision
                        if (Projectile.m_isplayer)
                        {
                            grid::Search(*this, *pShareFilter, X, Y, m_QueryEnemy, [&](entity& E, const position& Pos, sprite& Sprite, enemy& Enemy, timer& Timer, animation& Animator)  constexpr noexcept
                                {
                                    if (E.isZombie()) return false;
                                    if (Entity == E) return false;
                                    if (Projectile.m_ShipOwner == E) return false;

                                    if (Pos.m_Value.m_X - (Sprite.width / 2 * Sprite.size) < Position.m_Value.m_X && Position.m_Value.m_X < Pos.m_Value.m_X + (Sprite.width / 2 * Sprite.size))
                                    {
                                        if (Pos.m_Value.m_Y - (Sprite.height / 2 * Sprite.size) < Position.m_Value.m_Y && Position.m_Value.m_Y < Pos.m_Value.m_Y + (Sprite.height / 2 * Sprite.size))
                                        {
                                            s_Game.score += Enemy.score;
                                            s_Game.enemies--;
                                            DeleteEntity(Entity);
                                            
                                            Animator.animate = false;
                                            Enemy.isdead = true;
                                            Timer.active = true;
                                            Timer.m_Value = 0.0f;
                                            Timer.m_Timer = 0.1f;
                                            Sprite.data = Sprites::explodespritedata.sprite;
                                            Sprite.height = Sprites::explodespritedata.height;
                                            Sprite.width = Sprites::explodespritedata.width;

                                            return true;
                                        }
                                    }
                                    return false;
                                });
                        }
                        else
                        {
                            // Player - projectile collision
                            grid::Search(*this, *pShareFilter, X, Y, m_QueryPlayer, [&](entity& E, const position& Pos, sprite& Sprite)  constexpr noexcept
                                {
                                    if (E.isZombie()) return false;
                                    if (Entity == E) return false;
                                    if (Projectile.m_ShipOwner == E) return false;

                                    if (Pos.m_Value.m_X - (Sprite.width / 2 * Sprite.size) < Position.m_Value.m_X && Position.m_Value.m_X < Pos.m_Value.m_X + (Sprite.width / 2 * Sprite.size))
                                    {
                                        if (Pos.m_Value.m_Y - (Sprite.height / 2 * Sprite.size) < Position.m_Value.m_Y && Position.m_Value.m_Y < Pos.m_Value.m_Y + (Sprite.height / 2 * Sprite.size))
                                        {
                                            DeleteEntity(Entity);
                                            DeleteEntity(E);
                                            s_Game.lives--;
                                            if(s_Game.lives >= 0)
                                                Archetypes::GeneratePlayer();
                                            return true;
                                        }
                                    }
                                    return false;
                                });
                        }
                    });
            }
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
        if (!Animation.animate)
            return;

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

//---------------------------------------------------------------------------------------

struct update_ui : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::child_update<renderer, renderer::update>
    {
        .m_pName = "update_ui"
    };

    using query = std::tuple < xecs::query::must<manager> >;

    __inline
        void operator()(entity& Entity) const noexcept
    {
        int size = 5;
        glColor3f(1.0, 1.0, 1.0);
        GlutPrint(50, 50, "Score: %d", s_Game.score);
        GlutPrint(50, 25, "HighScore: %d", s_Game.m_Highscore);
        GlutPrint(s_Game.m_W/2 - 100, 50, "SPACE INVADERS");
    }
};

//---------------------------------------------------------------------------------------

struct render_border : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::child_update<renderer, renderer::update>
    {
        .m_pName = "render_border"
    };

    using query = std::tuple < xecs::query::must<manager> >;

    void OnPreUpdate(void) noexcept
    {
        glBegin(GL_QUADS);
    }

    void OnPostUpdate(void) noexcept
    {
        glEnd();
    }

    __inline
        void operator()(entity& Entity) const noexcept
    {
        constexpr auto Size = 3;

        glColor3f(0.3, 1.0, 0.5);

        // UI Bar Top
        glVertex2i(s_Game.m_W, s_Game.m_H / 12 * 1.8f - Size);
        glVertex2i(s_Game.m_W, s_Game.m_H / 12 * 1.8f + Size);
        glVertex2i(0, s_Game.m_H / 12 * 1.8f + Size);
        glVertex2i(0, s_Game.m_H / 12 * 1.8f - Size);
        // UI Bar Bottom
        glVertex2i(s_Game.m_W, s_Game.m_H / 12 * 11.5f - Size);
        glVertex2i(s_Game.m_W, s_Game.m_H / 12 * 11.5f + Size);
        glVertex2i(0, s_Game.m_H / 12 * 11.5f + Size);
        glVertex2i(0, s_Game.m_H / 12 * 11.5f - Size);
    }
};

//---------------------------------------------------------------------------------------

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
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) - Sprite.size / 2,
                        (Position.m_Value.m_Y - (Sprite.height * Sprite.size) / 2) + (y * Sprite.size) - Sprite.size / 2);
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) - Sprite.size / 2,
                        (Position.m_Value.m_Y - (Sprite.height * Sprite.size) / 2) + (y * Sprite.size) + Sprite.size / 2);
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) + Sprite.size / 2,
                        (Position.m_Value.m_Y - (Sprite.height * Sprite.size) / 2) + (y * Sprite.size) + Sprite.size / 2);
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) + Sprite.size / 2,
                        (Position.m_Value.m_Y - (Sprite.height * Sprite.size) / 2) + (y * Sprite.size) - Sprite.size / 2);
                }

            }
        }

        for (int i = 0; i < s_Game.lives; ++i)
        {
            for (int y = 0; y < Sprite.height; ++y)
            {
                for (int x = 0; x < Sprite.width; ++x)
                {
                    if (Sprite.data[x + (Sprite.width * y)])
                    {   // Position + enemy size
                        glVertex2i(((s_Game.m_W / 16 * 12) + (50 * i) - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) - Sprite.size / 2,
                            (60 - (Sprite.height * Sprite.size) / 2) + (y * Sprite.size) - Sprite.size / 2);
                        glVertex2i(((s_Game.m_W / 16 * 12) + (50 * i) - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) - Sprite.size / 2,
                            (60 - (Sprite.height * Sprite.size) / 2) + (y * Sprite.size) + Sprite.size / 2);
                        glVertex2i(((s_Game.m_W / 16 * 12) + (50 * i) - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) + Sprite.size / 2,
                            (60 - (Sprite.height * Sprite.size) / 2) + (y * Sprite.size) + Sprite.size / 2);
                        glVertex2i(((s_Game.m_W / 16 * 12) + (50 * i) - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) + Sprite.size / 2,
                            (60 - (Sprite.height * Sprite.size) / 2) + (y * Sprite.size) - Sprite.size / 2);
                    }

                }
            }
        }
    }
};

//---------------------------------------------------------------------------------------

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
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Sprite.size)/2) + (x * Sprite.size) - Sprite.size /2,
                        (Position.m_Value.m_Y - (Sprite.height * Sprite.size) / 2) + (y * Sprite.size) - Sprite.size /2);
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) - Sprite.size /2,
                        (Position.m_Value.m_Y - (Sprite.height * Sprite.size) / 2) + (y * Sprite.size) + Sprite.size /2);
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) + Sprite.size /2,
                        (Position.m_Value.m_Y - (Sprite.height * Sprite.size) / 2) + (y * Sprite.size) + Sprite.size /2);
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) + Sprite.size /2,
                        (Position.m_Value.m_Y - (Sprite.height * Sprite.size) / 2) + (y * Sprite.size) - Sprite.size /2);
                }

            }
        }
    }
};

//---------------------------------------------------------------------------------------

struct render_shield : xecs::system::instance
{
    constexpr static auto typedef_v = xecs::system::type::child_update<renderer, renderer::update>
    {
        .m_pName = "render_shield"
    };

    using query = std::tuple
        <
        xecs::query::must<shield>
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
        glColor3f(0.3, 1.0, 0.5);

        for (int y = 0; y < Sprite.height; ++y)
        {
            for (int x = 0; x < Sprite.width; ++x)
            {
                if (Sprite.data[x + (Sprite.width * y)])
                {
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) - Sprite.size / 2,
                        (Position.m_Value.m_Y - (Sprite.height * Sprite.size) / 2) + (y * Sprite.size) - Sprite.size / 2);
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) - Sprite.size / 2,
                        (Position.m_Value.m_Y - (Sprite.height * Sprite.size) / 2) + (y * Sprite.size) + Sprite.size / 2);
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) + Sprite.size / 2,
                        (Position.m_Value.m_Y - (Sprite.height * Sprite.size) / 2) + (y * Sprite.size) + Sprite.size / 2);
                    glVertex2i((Position.m_Value.m_X - (Sprite.width * Sprite.size) / 2) + (x * Sprite.size) + Sprite.size / 2,
                        (Position.m_Value.m_Y - (Sprite.height * Sprite.size) / 2) + (y * Sprite.size) - Sprite.size / 2);
                }

            }
        }
    }
};

//---------------------------------------------------------------------------------------

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