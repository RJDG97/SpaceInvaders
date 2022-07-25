namespace xecs::system
{
    namespace type
    {
        using guid = xcore::guid::unit<64, struct system_tag>;

        enum class id : std::uint8_t
        {
            UPDATE
        ,   NOTIFY_CREATE
        ,   NOTIFY_DESTROY
        ,   NOTIFY_MODIFIED
        ,   NOTIFY_MOVE_IN
        ,   NOTIFY_MOVE_OUT
        ,   NOTIFY_COMPONENT_CHANGE
        ,   NOTIFY_COMPONENT_ADDED
        ,   NOTIFY_COMPONENT_REMOVE
        ,   POOL_FAMILY_CREATE
        ,   POOL_FAMILY_DESTROY
        ,   GLOBAL_EVENT
        ,   SYSTEM_EVENT
        };

        struct update
        {
            static constexpr auto       id_v                = id::UPDATE;
            static constexpr auto       is_notifier_v       = false;
            const char*                 m_pName             = "Unnamed Update System";
            guid                        m_Guid              {};
        };

        struct notify_create
        {
            static constexpr auto       id_v                = id::NOTIFY_CREATE;
            static constexpr auto       is_notifier_v       = true;
            const char*                 m_pName             = "Unnamed Notified Create Entity System";
            guid                        m_Guid              {};
        };

        struct notify_destroy
        {
            static constexpr auto       id_v                = id::NOTIFY_DESTROY;
            static constexpr auto       is_notifier_v       = true;
            const char*                 m_pName             = "Unnamed Notified Destroy Entity System";
            guid                        m_Guid              {};
        };

        struct notify_moved_in
        {
            static constexpr auto       id_v                = id::NOTIFY_MOVE_IN;
            static constexpr auto       is_notifier_v       = true;
            const char*                 m_pName             = "Unnamed Notified Move In Entity System";
            guid                        m_Guid              {};
        };

        struct notify_moved_out
        {
            static constexpr auto       id_v                = id::NOTIFY_MOVE_OUT;
            static constexpr auto       is_notifier_v       = true;
            const char*                 m_pName             = "Unnamed Notified Move Out System";
            guid                        m_Guid              {};
        };

        struct notify_component_change
        {
            static constexpr auto        id_v               = id::NOTIFY_COMPONENT_CHANGE;
            static constexpr auto        is_notifier_v      = true;
            const char*                  m_pName            = "Unnamed Component Change System";
            guid                         m_Guid             {};
      const xecs::component::type::info* m_pComponentInfo   {};
        };

        struct pool_family_create
        {
            static constexpr auto       id_v                = id::POOL_FAMILY_CREATE;
            static constexpr auto       is_notifier_v       = true;
            const char*                 m_pName             = "Unnamed Pool Family Create System";
            guid                        m_Guid              {};
        };

        struct pool_family_destroy
        {
            static constexpr auto       id_v                = id::POOL_FAMILY_DESTROY;
            static constexpr auto       is_notifier_v       = true;
            const char*                 m_pName             = "Unnamed Pool Family Create System";
            guid                        m_Guid              {};
        };

        template< typename T_EVENT >
        requires( std::derived_from< T_EVENT, xecs::event::overrides> )
        struct global_event
        {
            using                        event_t            = T_EVENT;
            static constexpr auto        id_v               = id::GLOBAL_EVENT;
            static constexpr auto        is_notifier_v      = false;
            const char*                  m_pName            = "Unnamed Global Event System Delegate";
            guid                         m_Guid             {};
        };

        template< typename T_SYSTEM, typename T_EVENT >
        requires( std::derived_from< T_SYSTEM, xecs::system::instance>
                  && ( xcore::types::is_specialized_v<xecs::event::instance, T_EVENT>
                       || std::derived_from< T_EVENT, xecs::event::overrides>) )
        struct system_event
        {
            using                        system_t           = T_SYSTEM;
            using                        event_t            = T_EVENT;
            static constexpr auto        id_v               = id::SYSTEM_EVENT;
            static constexpr auto        is_notifier_v      = false;
            const char*                  m_pName            = "Unnamed System Event Delegate";
            guid                         m_Guid             {};
        };

        template< typename T_SYSTEM, typename T_EVENT >
        requires( std::derived_from< T_SYSTEM, xecs::system::instance>
                  && ( xcore::types::is_specialized_v<xecs::event::instance, T_EVENT>
                       || std::derived_from< T_EVENT, xecs::event::overrides>) )
        struct child_update
        {
            using                        system_t           = T_SYSTEM;
            using                        event_t            = T_EVENT;
            static constexpr auto        id_v               = id::SYSTEM_EVENT;
            static constexpr auto        is_notifier_v      = false;
            const char*                  m_pName            = "Unnamed Child Update Delegate";
            guid                         m_Guid             {};
        };

        struct info
        {
            using notifier_registration = void( xecs::archetype::instance&, xecs::system::instance&);

            const type::guid                        m_Guid;
            mutable xecs::query::instance           m_Query;
            notifier_registration* const            m_NotifierRegistration;
            const char* const                       m_pName;
            const id                                m_ID;
        };

        namespace details
        {
            template< typename T >
            consteval type::info CreateInfo(void) noexcept;

            template< typename T >
            static constexpr auto info_v = CreateInfo<T>();
        }

        template< typename T_SYSTEM >
        constexpr static auto& info_v = details::info_v<T_SYSTEM>;
    }

    //-----------------------------------------------------------------
    // SYSTEM OVERRIDES
    //-----------------------------------------------------------------
    struct overrides
    {
        using                   entity      = xecs::component::entity;      // Shortcut for entity
        using                   query       = std::tuple<>;                 // Override this to specify the query for the system
        using                   events      = std::tuple<>;                 // Override this to create events which other systems can use
        constexpr static auto   typedef_v   = type::update{};               // Override this to specify which type of system this is

        void    OnCreate                ( void )                noexcept {} // All Systems:         When the system is created
        void    OnGameStart             ( void )                noexcept {} // All Systems:         When the game starts or when it becomes unpaused
        void    OnFrameStart            ( void )                noexcept {} // All Systems:         At the begging of a frame
        void    OnPreUpdate             ( void )                noexcept {} // Update Systems:      Is call just before the OnUpdate gets call
        void    OnUpdate                ( void )                noexcept {} // Update Systems:      If you want full control of the query
        void    OnPostUpdate            ( void )                noexcept {} // Update Systems:      Is call just after the OnUpdate gets call and before structural changes happen
        void    OnPostStructuralChanges ( void )                noexcept {} // Update Systems:      After the Structural changes has taken place (applies only to )
        void    OnFrameEnd              ( void )                noexcept {} // All Systems:         Beginning of every frame
        void    OnGameEnd               ( void )                noexcept {} // All Systems:         When the game is done
        void    OnDestroy               ( void )                noexcept {} // All Systems:         Before destroying the system
        void    OnGamePause             ( void )                noexcept {} // All Systems:         When the game is paused 
        void    OnEvent                 ( ...  )                noexcept {} // Event System:        User overrides to receive the event message
        void    OnNotify                ( entity& Entity )      noexcept {} // Notify Systems:      Advance control
        void    OnPoolFamily            ( archetype::instance&              // Pool Family System:  Gets notified when a pool is created or destroy
                                        , pool::family&       ) noexcept {}
    };

    //-----------------------------------------------------------------
    // SYSTEM INSTANCE
    //-----------------------------------------------------------------
    struct instance : overrides
    {
                                            instance                ( const instance&
                                                                    ) noexcept = delete;

        constexpr                           instance                ( xecs::game_mgr::instance& GameMgr
                                                                    ) noexcept;

        template
        < typename   T_EVENT
        , typename   T_CLASS
        , typename...T_ARGS
        > requires
        ( std::derived_from<T_CLASS, xecs::system::instance>
            && (false == std::is_same_v<typename T_CLASS::events, xecs::system::overrides::events>)
            && !!(xcore::types::tuple_t2i_v<T_EVENT, typename T_CLASS::events > +1)
        ) __inline constexpr
        static void                         SendEventFrom           ( T_CLASS* pThis
                                                                    , T_ARGS&&... Args
                                                                    ) noexcept;
        template
        < typename      T_GLOBAL_EVENT
        > requires
        ( std::derived_from< T_GLOBAL_EVENT, xecs::event::overrides>
        ) __inline constexpr
        T_GLOBAL_EVENT&                     getGlobalEvent          ( void
                                                                    ) const noexcept;
        template
        < typename      T_GLOBAL_EVENT
        , typename...   T_ARGS
        > requires 
        ( std::derived_from< T_GLOBAL_EVENT, xecs::event::overrides>
        ) __inline constexpr
        void                                SendGlobalEvent         ( T_ARGS&&... Args 
                                                                    ) const noexcept;
        template
        < typename... T_TUPLES_OF_COMPONENTS_OR_COMPONENTS
        > requires
        ( 
            (   (  xecs::tools::valid_tuple_components_v<T_TUPLES_OF_COMPONENTS_OR_COMPONENTS>
                || xecs::component::type::is_valid_v<T_TUPLES_OF_COMPONENTS_OR_COMPONENTS> 
                ) &&... )
        ) __inline constexpr
        archetype::instance&                getOrCreateArchetype    ( void 
                                                                    ) const noexcept;
        template
        < typename... T_COMPONENTS
        > __inline constexpr
        [[nodiscard]] std::vector<archetype::instance*>
                                            Search                  ( const xecs::query::instance& Query
                                                                    ) const noexcept;
        template
        <   typename T_TUPLE_ADD
        ,   typename T_TUPLE_SUBTRACT   = std::tuple<>
        ,   typename T_FUNCTION         = xecs::tools::empty_lambda
        > requires
        ( xcore::function::is_callable_v<T_FUNCTION>
        && xcore::types::is_specialized_v<std::tuple, T_TUPLE_ADD>
        && xcore::types::is_specialized_v<std::tuple, T_TUPLE_SUBTRACT>
        ) __inline constexpr
        [[nodiscard]] xecs::component::entity
                                            AddOrRemoveComponents   ( xecs::component::entity   Entity
                                                                    , T_FUNCTION&&              Function = xecs::tools::empty_lambda{}
                                                                    ) const noexcept;
        __inline
        void                                DeleteEntity            ( xecs::component::entity& Entity 
                                                                    ) const noexcept;
        template
        <   typename T_FUNCTION
        ,   auto     T_SHARE_AS_DATA = false
        > requires
        ( xecs::tools::assert_is_callable_v<T_FUNCTION>
            && (   xecs::tools::function_return_v<T_FUNCTION, bool >
                || xecs::tools::function_return_v<T_FUNCTION, void > )
        ) __inline constexpr
            bool                            Foreach                 ( std::span<xecs::archetype::instance* const>   List
                                                                    , T_FUNCTION&&                                  Function 
                                                                    ) const noexcept;
        template
        <   typename T_FUNCTION
        > requires
        ( xecs::tools::assert_is_callable_v<T_FUNCTION>
            && (   xecs::tools::function_return_v<T_FUNCTION, bool >
                || xecs::tools::function_return_v<T_FUNCTION, void > )
        ) __inline
        bool                                Foreach                 ( const xecs::component::share_filter&  ShareFilter
                                                                    , const xecs::query::instance&          Query
                                                                    , T_FUNCTION&&                          Function 
                                                                    ) noexcept;
        template
        < typename T_SYSTEM
        > __inline constexpr
        T_SYSTEM*                           findSystem              ( void
                                                                    ) const noexcept;
        template
        < typename T_SYSTEM
        > __inline constexpr
        T_SYSTEM&                           getSystem               ( void
                                                                    ) const noexcept;
        template
        < typename T_SHARE_COMPONENT
        > __inline
        const xecs::component::share_filter*
                                            findShareFilter         ( T_SHARE_COMPONENT&&       ShareComponent
                                                                    , xecs::archetype::guid     ArchetypeGuid = xecs::archetype::guid{}
                                                                    ) noexcept;
        __inline
        const xecs::component::share_filter*
                                            findShareFilter         ( xecs::component::type::share::key Key
                                                                    ) noexcept;
    private:

        xecs::game_mgr::instance&   m_GameMgr;

        template< typename T_USER_SYSTEM >
        requires( std::derived_from< T_USER_SYSTEM, xecs::system::instance > )
        friend struct details::compleated;
    };
}