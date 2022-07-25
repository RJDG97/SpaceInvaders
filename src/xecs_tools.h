namespace xecs::tools
{
    using empty_lambda = decltype([]() {});

    //------------------------------------------------------------------------------

    template
    < typename T
    > concept
    assert_is_callable_v
    = []() constexpr noexcept
    {
        static_assert( xcore::function::is_callable_v<T>, "This is not callable (not a function or an object with operator())" );
        return true;
    }();
    
    //------------------------------------------------------------------------------

    template
    < typename T_CALLABLE
    , typename T_RETURN_TYPE
    > concept
    function_return_v
    = assert_is_callable_v<T_CALLABLE>
    && std::is_same_v<typename xcore::function::traits<T_CALLABLE>::return_type, T_RETURN_TYPE>;

    //------------------------------------------------------------------------------

    template
    < typename T_TUPLE_OF_COMPONENTS
    > concept
    valid_tuple_components_v
    = []() constexpr noexcept
    {
        if constexpr ( xcore::types::is_specialized_v< std::tuple, T_TUPLE_OF_COMPONENTS> )
        {
            return []<typename...T_ARGS>(std::tuple<T_ARGS...>*) constexpr noexcept
            {
                return ((xecs::component::type::is_valid_v<T_ARGS>) && ...);
            }( xcore::types::null_tuple_v< T_TUPLE_OF_COMPONENTS> );
        }
        else
        {
            return false;
        }
    }();

    //------------------------------------------------------------------------------

    template
    < typename T_TUPLE_OF_COMPONENTS
    > concept
    assert_valid_tuple_components_v
    = []<typename...T_ARGS>(std::tuple<T_ARGS...>*) constexpr noexcept
    {
        static_assert( ((xecs::component::type::is_valid_v<T_ARGS>) && ... ), "You have an element which is not a component" );
        return true;
    }( xcore::types::null_tuple_v< T_TUPLE_OF_COMPONENTS> );

    //------------------------------------------------------------------------------
    template
    < typename T_CALLABLE
    > concept
    assert_standard_function_v
    = assert_is_callable_v<T_CALLABLE>
    && []<typename...T_ARGS>(std::tuple<T_ARGS...>*) constexpr noexcept
    {
        static_assert( ((xecs::component::type::is_valid_v<T_ARGS>) && ... ), "You have a type in your function call that is not a valid component" );
        static_assert( false == xcore::types::tuple_has_duplicates_v< std::tuple< xcore::types::decay_full_t<T_ARGS> ... > >, "Found duplicated types in the function which is not allowed");
        static_assert( ((xecs::component::type::info_v<T_ARGS>.m_TypeID != xecs::component::type::id::TAG) && ...), "I found a tag in the parameter list of the function that is illegal");
        return true;
    }( xcore::types::null_tuple_v< typename xcore::function::template traits<T_CALLABLE>::args_tuple> );

    //------------------------------------------------------------------------------
    template
    < typename T_CALLABLE
    > concept
    assert_standard_setter_function_v
    = assert_standard_function_v<T_CALLABLE>
    && []<typename...T_ARGS>(std::tuple<T_ARGS...>*) constexpr noexcept
    {
        static_assert(((std::is_same_v< T_ARGS, std::remove_const_t<T_ARGS>>) && ...), "You can not have const in the parameters");
        static_assert( (( std::is_reference_v<T_ARGS>) && ...), "All the parameters of a setter should be references, we found at least one that it was not");
        return true;
    }( xcore::types::null_tuple_v< typename xcore::function::template traits<T_CALLABLE>::args_tuple> );


    //------------------------------------------------------------------------------
    template
    < typename T_CALLABLE
    > concept
    function_has_share_component_args_v
    = assert_standard_function_v<T_CALLABLE>
    && []<typename...T_ARGS>(std::tuple<T_ARGS...>*) constexpr noexcept
    {
        return ((xecs::component::type::info_v<T_ARGS>.m_TypeID == xecs::component::type::id::SHARE) || ...);
    }( xcore::types::null_tuple_v< typename xcore::function::template traits<T_CALLABLE>::args_tuple> );

    //------------------------------------------------------------------------------

    template
    < typename T_CALLABLE
    , typename T_RETURN_TYPE
    > concept
    assert_function_return_v
    = []() constexpr noexcept
    { static_assert( function_return_v< T_CALLABLE, T_RETURN_TYPE >, "Unacceptable return type of the given function" );
          return true;
    }();

    //------------------------------------------------------------------------------

    template
    < typename T_CALLABLE
    > concept
    assert_function_args_have_no_share_or_tag_components_v
    = assert_standard_function_v<T_CALLABLE>
    && []<typename...T_ARGS>(std::tuple<T_ARGS...>*) constexpr noexcept
    {
        static_assert( ((xecs::component::type::info_v<T_ARGS>.m_TypeID != xecs::component::type::id::SHARE) && ...), "You should not include share components in the argument of the callback" );
        return true;
    }( xcore::types::null_tuple_v< typename xcore::function::template traits<T_CALLABLE>::args_tuple> );
    
    //------------------------------------------------------------------------------

    template
    < typename T_CALLABLE
    > concept
    assert_function_args_have_only_non_const_references_v
    = assert_standard_setter_function_v<T_CALLABLE>;

    //------------------------------------------------------------------------------

    template
    < typename... T_SHARE_COMPONENTS
    > concept
    all_components_are_share_types_v
    = assert_standard_function_v<decltype([](T_SHARE_COMPONENTS...){})>
    && []() constexpr noexcept
    {
        return ((xecs::component::type::info_v<T_SHARE_COMPONENTS>.m_TypeID == xecs::component::type::id::SHARE) && ...);
    }();

    //------------------------------------------------------------------------------

    template
    < typename... T_SHARE_COMPONENTS
    > concept
    assert_all_components_are_share_types_v
    = []() constexpr noexcept
    {
        static_assert(all_components_are_share_types_v<T_SHARE_COMPONENTS...> );
        return true;
    }();
        
    //------------------------------------------------------------------------------

    namespace details
    {
        template< typename T>
        struct as_tuple
        {
            using type = std::tuple<T>;
        };

        template< typename...T >
        struct as_tuple<std::tuple<T...>>
        {
            using type = std::tuple<T...>;
        };
    }
    template< typename... T >
    using united_tuple = xcore::types::tuple_cat_t< typename details::as_tuple<T>::type ... >;
}
