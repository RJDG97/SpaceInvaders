namespace xecs::query
{
    namespace details
    {
        enum class mode
        {
            DEFAULT
        ,   DATA_ONLY
        ,   DATA_AND_ONLY_CONST_SHARES
        ,   DATA_AND_SHARES
        };

        //------------------------------------------------------------------------------------

        template
        < typename  T_FUNCTION
        , mode      T_HAS_SHARES_V = xecs::tools::function_has_share_component_args_v<T_FUNCTION> ? mode::DATA_AND_SHARES : mode::DATA_ONLY
        >
        struct data_pack;

        //------------------------------------------------------------------------------------

        template< typename T_FUNCTION>
        struct data_pack< T_FUNCTION, mode::DATA_ONLY >
        {
            static_assert(xcore::function::is_callable_v<T_FUNCTION>);

            using                   func                = xcore::function::traits<T_FUNCTION>;
            static constexpr auto   mode_v              = mode::DATA_ONLY;
            using                   data_tuple_unfilter = typename func::args_tuple;
            template< typename T >
            using                   universal_t         = xcore::types::decay_full_t<T>*;
            using                   data_tuple          = std::invoke_result_t
            < decltype
                (   []<typename...T>(std::tuple<T...>*) -> xcore::types::tuple_cat_t< std::tuple<universal_t<T>> ... >{}
                )
            , decltype(xcore::types::null_tuple_v<typename func::args_tuple>)
            >;

            archetype::instance*    m_pArchetype    {};
            pool::instance*         m_pPool         {};
            data_tuple              m_DataTuple;
        };

        //------------------------------------------------------------------------------------

        template< typename T_FUNCTION>
        struct data_pack< T_FUNCTION, mode::DATA_AND_SHARES >
        {
            static_assert(xcore::function::is_callable_v<T_FUNCTION>);

            using func = xcore::function::traits<T_FUNCTION>;

            static constexpr auto mode_v = mode::DATA_AND_SHARES;

            using share_tuple_unfilter              = std::invoke_result_t
            < decltype
                (   []<typename...T>(std::tuple<T...>*)->xcore::types::tuple_cat_t
                    <
                        std::conditional_t
                        < xecs::component::type::info_v<T>.m_TypeID == xecs::component::type::id::SHARE
                        , std::tuple<T>
                        , std::tuple<>
                        > ...
                    >{}
                )
            , decltype(xcore::types::null_tuple_v<typename func::args_tuple>)
            >;

            using share_tuple_full_decay            = xcore::types::tuple_decay_full_t<share_tuple_unfilter>;
            
            using share_tuple_changables_unfilter   = std::invoke_result_t
            < decltype
                (   []<typename...T>(std::tuple<T...>*) -> xcore::types::tuple_cat_t
                    <
                        std::conditional_t
                        < std::is_const_v<T>
                        , std::tuple<>
                        , std::tuple<T>
                        > ...
                    >{}
                )
            , decltype(xcore::types::null_tuple_v<share_tuple_unfilter>)
            >;

            static constexpr auto share_count_v             = std::tuple_size_v<share_tuple_unfilter>;
            static constexpr auto nonconst_share_count_v    = std::tuple_size_v<share_tuple_changables_unfilter>;
            static constexpr auto const_share_count_v       = share_count_v - nonconst_share_count_v;
            static constexpr auto data_count_v              = func::arg_count_v - share_count_v;

            static_assert(share_count_v);


            using data_tuple_unfilter = std::invoke_result_t
            < decltype
                (   []<typename...T>(std::tuple<T...>*) -> xcore::types::tuple_cat_t
                    <
                        std::conditional_t
                        < xecs::component::type::info_v<T>.m_TypeID == xecs::component::type::id::DATA
                        , std::tuple<T>
                        , std::tuple<>
                        > ...
                    >{}
                )
            , decltype(xcore::types::null_tuple_v<typename func::args_tuple>)
            >;

            template< typename T >
            using universal_t = std::conditional_t
            <   xecs::component::type::info_v<T>.m_TypeID == xecs::component::type::id::DATA
            ,   xcore::types::decay_full_t<T>*
            ,   std::conditional_t
                <   std::is_reference_v<T>
                ,   std::conditional_t
                    <   std::is_const_v<T>
                    ,   std::remove_reference_t<T>*
                    ,   std::remove_reference_t<T>
                    >
                ,   std::conditional_t
                    <   std::is_const_v<T>
                    ,   T
                    ,   std::remove_pointer_t<T>
                    >
                >
            >;

            using data_tuple = std::invoke_result_t
            < decltype
                (   []<typename...T>(std::tuple<T...>*) -> xcore::types::tuple_cat_t< std::tuple<universal_t<T>> ... >{}
                )
            , decltype(xcore::types::null_tuple_v<typename func::args_tuple>)
            >;

            using share_tuple = std::invoke_result_t
            < decltype
                (   []<typename...T>(std::tuple<T...>*) -> xcore::types::tuple_cat_t
                    <
                        std::conditional_t
                        < xecs::component::type::info_v<T>.m_TypeID == xecs::component::type::id::SHARE
                        , std::tuple<universal_t<T>>
                        , std::tuple<>
                        > ...
                    >{}
                )
            , decltype(xcore::types::null_tuple_v<typename func::args_tuple>)
            >;

            using share_tuple_pointers = std::invoke_result_t
            < decltype
                (   []<typename...T>(std::tuple<T...>*) -> xcore::types::tuple_cat_t
                    <
                        std::conditional_t
                        < xecs::component::type::info_v<T>.m_TypeID == xecs::component::type::id::SHARE && (false == std::is_pointer_v<universal_t<T>>)
                        , std::tuple<xcore::types::decay_full_t<T>*>
                        , std::tuple<>
                        > ...
                    >{}
                )
            , decltype(xcore::types::null_tuple_v<typename func::args_tuple>)
            >;
            static_assert(nonconst_share_count_v == std::tuple_size_v<share_tuple_pointers>);


            __inline data_pack() noexcept;

            game_mgr::instance*                                                     m_pGameMgr                  {};
            archetype::instance*                                                    m_pArchetype                {};
            pool::family*                                                           m_pFamily                   {};
            pool::instance*                                                         m_pPool                     {};
            const xecs::tools::bits                                                 m_UpdatedComponentsBits;
            xecs::tools::bits                                                       m_ArchetypeShareBits;
            std::array<std::uint8_t,                        share_count_v>          m_RemapIndices;
            std::array<xecs::component::type::share::key,   share_count_v>          m_CacheShareKeys;
            share_tuple_pointers                                                    m_CacheSharePointers;
            share_tuple                                                             m_UniversalTuple;
            data_tuple                                                              m_DataTuple;
            std::uint64_t                                                           m_KeyCheckSum;
        };

        //------------------------------------------------------------------------------------

        template<typename T_FUNCTION, auto T_MODE_V >
        using choose_pack_t = std::conditional_t
        < T_MODE_V == mode::DEFAULT
        , data_pack<T_FUNCTION>
        , data_pack<T_FUNCTION, T_MODE_V>
        >;
    }

    //------------------------------------------------------------------------------------

    template< typename T_FUNCTION, auto T_MODE_V = details::mode::DEFAULT >
    struct iterator : details::choose_pack_t< T_FUNCTION, T_MODE_V >
    {
        using                   parent_t    = details::choose_pack_t< T_FUNCTION, T_MODE_V >;
        using                   func_t      = xcore::function::traits<T_FUNCTION>;
        using                   ret_t       = func_t::return_type;
        constexpr static auto   mode_v      = parent_t::mode_v;

        __inline
                    iterator                ( xecs::game_mgr::instance&     GameMgr
                                            ) noexcept;
        __inline
        void        UpdateArchetype         ( xecs::archetype::instance&    Archetype 
                                            ) noexcept;
        __inline
        void        UpdateFamilyPool        ( xecs::pool::family&           Family 
                                            ) noexcept;
        __inline
        void        UpdatePool              ( xecs::pool::instance&         Pool 
                                            ) noexcept;
        __inline
        ret_t       ForeachEntity           ( T_FUNCTION&&                  Function 
                                            ) noexcept;
    };
}