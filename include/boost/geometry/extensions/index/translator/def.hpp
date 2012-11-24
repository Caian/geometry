// Boost.Geometry Index
//
// Default translator
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_TRANSLATOR_DEF_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_TRANSLATOR_DEF_HPP

#include <boost/geometry/extensions/index/translator/helpers.hpp>

namespace boost { namespace geometry { namespace index { namespace translator {

//namespace dispatch {
//
//// Distinguish between def<Geometry>, def<Iterator> and def<SmartPtr>
//
//// Geometry
//template <typename Value, bool IsIterator, bool IsSmartPtr>
//struct def
//{
//    typedef typename detail::extract_indexable<Value>::type const& result_type;
//
//    result_type operator()(Value const& v) const
//    {
//        return detail::extract_indexable<Value>::get(v);
//    }
//
//    bool equals(Value const& v1, Value const& v2) const
//    {
//        return detail::equals<Value>::apply(v1, v2);
//    }
//};
//
//// Iterator
//template <typename Value, bool IsSmartPtr>
//struct def<Value, true, IsSmartPtr>
//{
//    typedef typename detail::extract_indexable<typename Value::value_type>::type const& result_type;
//
//    result_type operator()(Value const& v) const
//    {
//        return detail::extract_indexable<typename Value::value_type>::get(*v);
//    }
//
//    bool equals(Value const& v1, Value const& v2) const
//    {
//        return v1 == v2;
//    }
//};
//
//// SmartPtr
//template <typename Value>
//struct def<Value, false, true>
//{
//    typedef typename detail::extract_indexable<typename Value::element_type>::type const& result_type;
//
//    result_type operator()(Value const& v) const
//    {
//        return detail::extract_indexable<typename Value::element_type>::get(*v);
//    }
//
//    bool equals(Value const& v1, Value const& v2) const
//    {
//        return v1 == v2;
//    }
//};
//
//} // namespace dispatch
//
///*!
//The default translator. It translates Value object to Indexable object. This is done in
//operator() which takes const reference to Value and returns const reference to Indexable.
//
//\tparam Value       The Value type which the translator translates to Indexable.
//*/
//template <typename Value>
//struct def
//    : public dispatch::def
//        <
//            Value,
//            detail::is_iterator<Value>::value,
//            detail::is_smart_ptr<Value>::value
//        >
//{
//};
//
///*!
//The default translator. It translates Value object to Indexable object. Since this is
//a specialization for pointers to Values operator() takes const ptr to Value and returns
//const reference to Indexable.
//
//\tparam Value       The Value type which the translator translates to Indexable.
//*/
//template <typename Value>
//struct def<Value*>
//{
//    typedef typename detail::extract_indexable<Value>::type const& result_type;
//
//    result_type operator()(const Value *v) const
//    {
//        return detail::extract_indexable<Value>::get(*v);
//    }
//
//    bool equals(const Value* v1, const Value* v2) const
//    {
//        return v1 == v2;
//    }
//};

/*!
The default translator. It translates Value object to Indexable object.

\tparam Value       The Value type which may be translated directly to the Indexable.
*/
template <typename Value>
struct def
{
    BOOST_MPL_ASSERT_MSG(
        (!detail::indexable_not_found_error<
            typename traits::indexable_type<Value>::type
         >::value),
        NOT_VALID_INDEXABLE_TYPE,
        (Value)
    );

    typedef Value const& result_type;

    result_type operator()(Value const& value) const
    {
        return value;
    }

    bool equals(Value const& v1, Value const& v2) const
    {
        return geometry::equals(v1, v2);
    }
};

/*!
The default translator. This specialization translates from std::pair<Indexable, Second>.

\tparam Indexable       The Indexable type.
\tparam Second          The second type.
*/
template <typename Indexable, typename Second>
struct def< std::pair<Indexable, Second> >
{
    BOOST_MPL_ASSERT_MSG(
        (!detail::indexable_not_found_error<
            typename traits::indexable_type<Indexable>::type
         >::value),
        NOT_VALID_INDEXABLE_TYPE,
        (Indexable)
    );

    typedef Indexable const& result_type;

    result_type operator()(std::pair<Indexable, Second> const& value) const
    {
        return value.first;
    }

    bool equals(std::pair<Indexable, Second> const& v1, std::pair<Indexable, Second> const& v2) const
    {
        return geometry::equals(v1.first, v2.first)
            &&
            dispatch::equals<
                Second,
                typename geometry::traits::tag<Second>::type
            >::apply(v1.second, v2.second);
    }
};

}}}} // namespace boost::geometry::index::translator

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_TRANSLATOR_DEF_HPP
